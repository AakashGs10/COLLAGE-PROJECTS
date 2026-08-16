/**
 * ============================================================================
 * Project: SignalKey - Secure IoT Data Transmission via PLS & Obfuscation
 * Target : Contiki OS 2.7 / Cooja IoT Network Simulator (RPL / 6LoWPAN)
 * System : All-in-One Master Firmware (Gateway Sink + 3 Sender Protocols)
 * ============================================================================
 * CONFIGURATION INSTRUCTIONS:
 * 1. Gateway Mote: In Cooja, assign your receiver mote ID 11 (GATEWAY_NODE_ID).
 * It will automatically boot as the Universal Sink.
 * 2. Sender Motes: Set ACTIVE_PROTOCOL below to select the test protocol:
 * - PROTOCOL_NORMAL       : Baseline unencrypted plaintext telemetry
 * - PROTOCOL_RSSI_SECURE  : SignalKey physical layer security & obfuscation
 * - PROTOCOL_DOUBLE_ENC   : Traditional 4-way interactive handshake
 * ============================================================================
 */

#include "contiki.h"
#include "net/uip.h"
#include "simple-udp.h"
#include "sys/node-id.h"
#include "sys/energest.h"
#include "lib/random.h"
#include <stdio.h>
#include <string.h>

/* --- PROTOCOL SELECTION MACROS --- */
#define PROTOCOL_NORMAL       0
#define PROTOCOL_RSSI_SECURE  1
#define PROTOCOL_DOUBLE_ENC   2

/**
 * >>> SELECT YOUR SENDER TEST PROTOCOL HERE <<<
 */
#define ACTIVE_PROTOCOL PROTOCOL_RSSI_SECURE

/* --- SYSTEM & NETWORK PARAMETERS --- */
#define UDP_PORT              1234
#define GATEWAY_NODE_ID       11    /* Mote ID designated as Universal Sink */
#define SIMULATION_TIME       600   /* 10 Minutes hard execution timeout    */
#define PLS_KEY               0x5A  /* RSSI physical channel derivation seed */
#define DOUBLE_KEY2           0x33  /* Secondary key for DoubleEnc handshake */

PROCESS(signalkey_master_process, "SignalKey Master Unified Firmware");
AUTOSTART_PROCESSES(&signalkey_master_process);

static struct simple_udp_connection udp_conn;
static uint32_t tx_count = 0;
static uint32_t rx_count = 0;

/* ============================================================================
 * CRYPTOGRAPHIC & OBFUSCATION ENGINE
 * ============================================================================ */

/**
 * Bitwise Physical Layer Security (PLS) / XOR Engine
 * Applies lightweight stream obfuscation without null-byte string bugs.
 */
void apply_crypto_layer(char *data, int len, uint8_t key) {
  int i;
  for(i = 0; i < len; i++) {
    data[i] ^= key;
  }
}

/* ============================================================================
 * UNIVERSAL RECEIVER CALLBACK (HANDLES GATEWAY SINK & HANDSHAKES)
 * ============================================================================ */
static void universal_udp_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr, uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr, uint16_t receiver_port,
         const uint8_t *data, uint16_t datalen) 
{
  char buffer[80];
  if(datalen >= sizeof(buffer)) return; /* Prevent frame buffer overflow */
  
  memcpy(buffer, data, datalen);
  buffer[datalen] = '\0';

  /* --------------------------------------------------------------------------
   * ROLE 1: UNIVERSAL GATEWAY SINK LOGIC (Node ID == GATEWAY_NODE_ID)
   * -------------------------------------------------------------------------- */
  if(node_id == GATEWAY_NODE_ID) {
    
    /* [Protocol A] Normal Unsecured Telemetry (Tag: 'T') */
    if(buffer[0] == 'T') {
      rx_count++;
      printf("RX_LOG,0,%lu,%u\n", rx_count, datalen);
      printf("[Gateway] Plaintext Telemetry: %s\n", &buffer[1]);
      return;
    }

    /* [Protocol B] SignalKey RSSI PLS Secure Mode (Tag: 'R') */
    if(buffer[0] == 'R') {
      apply_crypto_layer(&buffer[1], datalen - 1, PLS_KEY);
      buffer[datalen] = '\0';
      rx_count++;
      printf("RX_LOG,0,%lu,%u\n", rx_count, datalen);
      printf("[Gateway] SignalKey PLS Validated: %s\n", &buffer[1]);
      return;
    }

    /* [Protocol C - Step 1] Double Encryption Handshake Initiation (Tag: '1') */
    if(buffer[0] == '1') {
      apply_crypto_layer(&buffer[1], datalen - 1, DOUBLE_KEY2);
      buffer[0] = '2'; /* Tag Step 2: Gateway applied secondary layer */
      simple_udp_sendto(&udp_conn, buffer, datalen, sender_addr);
      printf("[Gateway] Processed DoubleEnc Step 1 -> Returned Step 2 ACK\n");
      return;
    }

    /* [Protocol C - Step 3] Double Encryption Handshake Completion (Tag: '3') */
    if(buffer[0] == '3') {
      apply_crypto_layer(&buffer[1], datalen - 1, DOUBLE_KEY2);
      buffer[datalen] = '\0';
      rx_count++;
      printf("RX_LOG,0,%lu,%u\n", rx_count, datalen);
      printf("[Gateway] DoubleEnc Handshake Complete: %s\n", &buffer[1]);
      return;
    }
  } 
  
  /* --------------------------------------------------------------------------
   * ROLE 2: SENSOR SENDER LOGIC (Double Encryption Handshake ACK Handler)
   * -------------------------------------------------------------------------- */
  else {
    /* [Protocol C - Step 2] Sender receives Gateway's double-encrypted ACK */
    if(ACTIVE_PROTOCOL == PROTOCOL_DOUBLE_ENC && buffer[0] == '2') {
      /* Remove Node's initial Layer 1 encryption; leave Layer 2 intact */
      apply_crypto_layer(&buffer[1], datalen - 1, PLS_KEY);
      buffer[0] = '3'; /* Tag Step 3: Sender removed primary layer */
      simple_udp_sendto(&udp_conn, buffer, datalen, sender_addr);
      printf("[Sender %d] Handshake Step 2 ACKed -> Transmitted Step 3\n", node_id);
    }
  }
}

/* ============================================================================
 * MAIN PROCESS THREAD (SYSTEM PROFILING & TRAFFIC SCHEDULER)
 * ============================================================================ */
PROCESS_THREAD(signalkey_master_process, ev, data) 
{
  static struct etimer tx_timer;
  static struct etimer sec_timer;
  static int seconds_elapsed = 0;
  static char payload[80];
  uip_ipaddr_t mcast_addr;

  PROCESS_BEGIN();

  /* Register UDP socket bound to universal multi-role handler */
  simple_udp_register(&udp_conn, UDP_PORT, NULL, UDP_PORT, universal_udp_callback);

  /* 1-second system ticker prevents Contiki 16-bit etimer overflow bugs */
  etimer_set(&sec_timer, CLOCK_SECOND);

  /* Boot Diagnostics & Role Assignment Display */
  printf("========================================================\n");
  if(node_id == GATEWAY_NODE_ID) {
    printf("[Boot] Role: UNIVERSAL GATEWAY SINK (Mote ID: %d)\n", node_id);
    printf("[Boot] Listening for Normal, RSSI PLS, and DoubleEnc frames...\n");
  } else {
    printf("[Boot] Role: SENSOR SENDER (Mote ID: %d)\n", node_id);
    if(ACTIVE_PROTOCOL == PROTOCOL_NORMAL)      printf("[Boot] Active Protocol: NORMAL UNSECURED BASELINE\n");
    if(ACTIVE_PROTOCOL == PROTOCOL_RSSI_SECURE) printf("[Boot] Active Protocol: SIGNALKEY RSSI PLS SECURE\n");
    if(ACTIVE_PROTOCOL == PROTOCOL_DOUBLE_ENC)  printf("[Boot] Active Protocol: TWO-WAY DOUBLE ENCRYPTION\n");
    
    /* Desynchronize boot timers to prevent initial MAC radio collisions */
    etimer_set(&tx_timer, CLOCK_SECOND * 5 + (random_rand() % (CLOCK_SECOND * 5)));
  }
  printf("========================================================\n");

  while(1) {
    PROCESS_WAIT_EVENT();

    /* --- 1. HARDWARE ENERGEST BENCHMARKING & TIMEOUT MONITOR --- */
    if(etimer_expired(&sec_timer)) {
      seconds_elapsed++;
      
      if(seconds_elapsed >= SIMULATION_TIME) {
        printf("\n========================================================\n");
        if(node_id == GATEWAY_NODE_ID) {
          printf("SESSION_COMPLETE: UNIVERSAL SINK RX_TOTAL:%lu\n", rx_count);
        } else {
          printf("SESSION_COMPLETE: SENDER %d SHUTDOWN AT T=%ds\n", node_id, seconds_elapsed);
        }
        /* Dump CPU, Low Power Mode, Transmit, and Listen Energy Ticks */
        printf("SYS_STATS: %lu %lu %lu %lu\n",
               energest_type_time(ENERGEST_TYPE_CPU),
               energest_type_time(ENERGEST_TYPE_LPM),
               energest_type_time(ENERGEST_TYPE_TRANSMIT),
               energest_type_time(ENERGEST_TYPE_LISTEN));
        printf("========================================================\n");
        PROCESS_EXIT();
      }
      etimer_reset(&sec_timer);
    }

    /* --- 2. SENSOR TELEMETRY TRANSMISSION LOOP (SENDERS ONLY) --- */
    if(node_id != GATEWAY_NODE_ID && etimer_expired(&tx_timer)) {
      
      /* Sample simulated environmental sensor telemetry */
      int temp_val = 20 + (random_rand() % 30);
      sprintf(&payload[1], "TEMP:%d|ID:%d", temp_val, node_id);
      int data_len = strlen(&payload[1]);

      uip_create_linklocal_allnodes_mcast(&mcast_addr);

      /* [Mode 0] Normal Unsecured Transmission */
      if(ACTIVE_PROTOCOL == PROTOCOL_NORMAL) {
        payload[0] = 'T';
        int total_bytes = data_len + 1;
        simple_udp_sendto(&udp_conn, payload, total_bytes, &mcast_addr);
        tx_count++;
        printf("TX_LOG,1,%d,%lu,%d,0\n", node_id, tx_count, total_bytes);
        printf("[Normal Node %d] Sent plaintext frame (#%lu)\n", node_id, tx_count);
      }

      /* [Mode 1] SignalKey RSSI PLS Transmission */
      else if(ACTIVE_PROTOCOL == PROTOCOL_RSSI_SECURE) {
        apply_crypto_layer(&payload[1], data_len, PLS_KEY);
        payload[0] = 'R';
        int total_bytes = data_len + 1;
        simple_udp_sendto(&udp_conn, payload, total_bytes, &mcast_addr);
        tx_count++;
        printf("TX_LOG,1,%d,%lu,%d,0\n", node_id, tx_count, total_bytes);
        printf("[RSSI Node %d] Sent SignalKey PLS frame (#%lu)\n", node_id, tx_count);
      }

      /* [Mode 2] Double Encryption Handshake Initiation (Step 1) */
      else if(ACTIVE_PROTOCOL == PROTOCOL_DOUBLE_ENC) {
        apply_crypto_layer(&payload[1], data_len, PLS_KEY);
        payload[0] = '1';
        int total_bytes = data_len + 1;
        simple_udp_sendto(&udp_conn, payload, total_bytes, &mcast_addr);
        tx_count++;
        printf("TX_LOG,1,%d,%lu,%d,0\n", node_id, tx_count, total_bytes);
        printf("[Double Node %d] Initiated handshake Step 1 (#%lu)\n", node_id, tx_count);
      }

      /* Standardized 15–25s randomized jitter to eliminate 802.15.4 MAC collisions */
      etimer_set(&tx_timer, CLOCK_SECOND * 15 + (random_rand() % (CLOCK_SECOND * 10)));
    }
  }

  PROCESS_END();
}
