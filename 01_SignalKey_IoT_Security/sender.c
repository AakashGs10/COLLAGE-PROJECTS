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

PROCESS(sender_normal_process, "Normal Baseline Sender");
AUTOSTART_PROCESSES(&sender_normal_process);

static struct simple_udp_connection udp_conn;
static uint32_t tx_count = 0;

PROCESS_THREAD(sender_normal_process, ev, data) {
  static struct etimer tx_timer;
  static struct etimer sec_timer;
  static int seconds_elapsed = 0;
  static char payload[80];
  uip_ipaddr_t mcast_addr;

  PROCESS_BEGIN();

  simple_udp_register(&udp_conn, UDP_PORT, NULL, UDP_PORT, NULL);

  etimer_set(&sec_timer, CLOCK_SECOND);
  /* Standardized randomized timer: 15 to 25 seconds */
  etimer_set(&tx_timer, CLOCK_SECOND * 15 + (random_rand() % (CLOCK_SECOND * 10)));

  while(1) {
    PROCESS_WAIT_EVENT();

    /* --- System Clock & Hardware Power Profiling --- */
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

    /* --- Telemetry Transmission --- */
    if(etimer_expired(&tx_timer)) {
      int temp_val = 20 + (random_rand() % 30);
      sprintf(payload, "TEMP:%d", temp_val);
      
      uip_create_linklocal_allnodes_mcast(&mcast_addr);
      int total_bytes = strlen(payload) + 1;
      simple_udp_sendto(&udp_conn, payload, total_bytes, &mcast_addr);

      tx_count++;
      printf("TX_LOG,1,%d,%lu,%d,0\n", node_id, tx_count, total_bytes);
      printf("[Normal Node %d] Sent unencrypted packet (#%lu)\n", node_id, tx_count);

      etimer_set(&tx_timer, CLOCK_SECOND * 15 + (random_rand() % (CLOCK_SECOND * 10)));
    }
  }
  PROCESS_END();
}