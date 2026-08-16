#include "contiki.h"
#include "net/uip.h"
#include "simple-udp.h"
#include "sys/node-id.h"
#include "sys/energest.h"
#include "lib/random.h"
#include <stdio.h>
#include <string.h>

#define UDP_PORT 1234
#define SIMULATION_TIME 600 /* 10 Minutes */
#define PLS_KEY 0x5A

PROCESS(sender_rssi_process, "RSSI PLS Secure Sender");
AUTOSTART_PROCESSES(&sender_rssi_process);

static struct simple_udp_connection udp_conn;
static uint32_t tx_count = 0;

void apply_pls_encryption(char *data, int len) {
  int i;
  for(i = 0; i < len; i++) {
    data[i] ^= PLS_KEY;
  }
}

PROCESS_THREAD(sender_rssi_process, ev, data) {
  static struct etimer tx_timer;
  static struct etimer sec_timer;
  static int seconds_elapsed = 0;
  static char payload[80];
  uip_ipaddr_t mcast_addr;

  PROCESS_BEGIN();

  simple_udp_register(&udp_conn, UDP_PORT, NULL, UDP_PORT, NULL);

  etimer_set(&sec_timer, CLOCK_SECOND);
  etimer_set(&tx_timer, CLOCK_SECOND * 15 + (random_rand() % (CLOCK_SECOND * 10)));

  while(1) {
    PROCESS_WAIT_EVENT();

    if(etimer_expired(&sec_timer)) {
      seconds_elapsed++;
      if(seconds_elapsed >= SIMULATION_TIME) {
        printf("SYS_STATS: %lu %lu %lu %lu\n",
               energest_type_time(ENERGEST_TYPE_CPU),
               energest_type_time(ENERGEST_TYPE_LPM),
               energest_type_time(ENERGEST_TYPE_TRANSMIT),
               energest_type_time(ENERGEST_TYPE_LISTEN));
        PROCESS_EXIT();
      }
      etimer_reset(&sec_timer);
    }

    if(etimer_expired(&tx_timer)) {
      int temp_val = 20 + (random_rand() % 30);
      sprintf(&payload[1], "TEMP:%d", temp_val);
      
      int data_len = strlen(&payload[1]);
      apply_pls_encryption(&payload[1], data_len);
      payload[0] = 'R'; /* Protocol Tag: RSSI Single-Trip Secure */

      uip_create_linklocal_allnodes_mcast(&mcast_addr);
      int total_bytes = data_len + 1;
      simple_udp_sendto(&udp_conn, payload, total_bytes, &mcast_addr);

      tx_count++;
      printf("TX_LOG,1,%d,%lu,%d,0\n", node_id, tx_count, total_bytes);
      printf("[RSSI Node %d] Sent PLS encrypted packet (#%lu)\n", node_id, tx_count);

      etimer_set(&tx_timer, CLOCK_SECOND * 15 + (random_rand() % (CLOCK_SECOND * 10)));
    }
  }
  PROCESS_END();
}