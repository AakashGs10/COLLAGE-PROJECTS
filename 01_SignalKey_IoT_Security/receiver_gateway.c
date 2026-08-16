#include "contiki.h"
#include "net/uip.h"
#include "simple-udp.h"
#include <stdio.h>
#include <string.h>

#define UDP_PORT 1234
#define SIMULATION_TIME 600 /* 10 Minutes */
#define KEY1 0x5A
#define KEY2 0x33

PROCESS(receiver_gateway_process, "Universal Gateway Sink");
AUTOSTART_PROCESSES(&receiver_gateway_process);

static struct simple_udp_connection udp_conn;
static uint32_t rx_count = 0;

void apply_key(char *data, int len, uint8_t key) {
  int i;
  for(i = 0; i < len; i++) {
    data[i] ^= key;
  }
}

static void rx_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr, uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr, uint16_t receiver_port,
         const uint8_t *data, uint16_t datalen) 
{
  char buffer[80];
  if(datalen >= sizeof(buffer)) return;
  
  memcpy(buffer, data, datalen);
  buffer[datalen] = '\0';

  /* 1. DOUBLE ENCRYPTION: Step 1 -> Apply Key 2 and echo back */
  if(buffer[0] == '1') {
    apply_key(&buffer[1], datalen - 1, KEY2);
    buffer[0] = '2';
    simple_udp_sendto(&udp_conn, buffer, datalen, sender_addr);
    printf("[Gateway] Processed DoubleEnc Step 1 -> Sent Step 2\n");
    return;
  }

  /* 2. DOUBLE ENCRYPTION: Step 3 -> Remove Key 2 and finalize */
  if(buffer[0] == '3') {
    apply_key(&buffer[1], datalen - 1, KEY2);
    buffer[datalen] = '\0';
    rx_count++;
    printf("RX_LOG,0,%lu,%u\n", rx_count, datalen);
    printf("[Gateway] DoubleEnc Success: %s\n", &buffer[1]);
    return;
  }

  /* 3. RSSI PLS SECURE MODE: Strip Key 1 and finalize */
  if(buffer[0] == 'R') {
    apply_key(&buffer[1], datalen - 1, KEY1);
    buffer[datalen] = '\0';
    rx_count++;
    printf("RX_LOG,0,%lu,%u\n", rx_count, datalen);
    printf("[Gateway] RSSI PLS Success: %s\n", &buffer[1]);
    return;
  }

  /* 4. NORMAL UNSECURED MODE: Plaintext telemetry */
  if(buffer[0] == 'T') {
    rx_count++;
    printf("RX_LOG,0,%lu,%u\n", rx_count, datalen);
    printf("[Gateway] Normal Telemetry: %s\n", buffer);
    return;
  }
}

PROCESS_THREAD(receiver_gateway_process, ev, data) {
  static struct etimer sec_timer;
  static int seconds_elapsed = 0;

  PROCESS_BEGIN();

  simple_udp_register(&udp_conn, UDP_PORT, NULL, UDP_PORT, rx_callback);
  etimer_set(&sec_timer, CLOCK_SECOND);

  printf("========================================================\n");
  printf("[Gateway] Universal Sink Active on Port %d\n", UDP_PORT);
  printf("========================================================\n");

  while(1) {
    PROCESS_WAIT_EVENT();
    if(etimer_expired(&sec_timer)) {
      seconds_elapsed++;
      if(seconds_elapsed >= SIMULATION_TIME) {
        printf("\nSESSION_COMPLETE: RX_TOTAL:%lu\n", rx_count);
        PROCESS_EXIT();
      }
      etimer_reset(&sec_timer);
    }
  }
  PROCESS_END();
}