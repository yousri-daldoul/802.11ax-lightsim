#include <stdio.h>
#include <stdlib.h>     /* srand, rand */
#include "he_wlan.h"
#include <math.h>       /* ceil */


/*
This function measures the collision rate of pure UL OFDMA as a function of the number of contending stations and the number of RA RUs.
It is used to build Figure 6 of the paper (Performance Evaluation of OFDMA and MU-MIMO in 802.11ax Networks)
*/
void pureOfdmaCollisionRate() {
	struct wlan_result result;
	
	printf("Collision rate of pure UL OFDMA (1x1 UL MU-MIMO):\n");
	printf("nRAStas    1RARUs    2RARUs    4RARUs    8RARUs   16RARUs\n");
	
	setApAntennas(1);
	
	int step = 1;
	for (int nRAStas = 1; nRAStas <= 100; nRAStas += step) {
		if (nRAStas == 10) step = 10;
		
		printf("%7d", nRAStas);
		for (int nRARUs = 1; nRARUs <= 16; nRARUs *= 2) {
			result = simulate_wlan(BANDWIDTH_80MHz, PURE_UL_OFDMA, RU_SIZE_52_TONES, nRARUs, nRAStas, MCS_6, 100);
			printf("    %5.2f%%", result.ofdma_collision_rate);
		}
		printf("\n");
	}
}


/*
This function measures the throughput of pure UL OFDMA as a function of the number of contending stations and the number of RA RUs.
It is used to build Figure 7 of the paper
*/
void pureOfdmaThroughput() {
	struct wlan_result result;
	
	printf("Throughput of pure UL OFDMA (1x1 UL MU-MIMO):\n");
	printf("we measure the throughput as a function of the number of contending stations (not all stations)\n");
	printf("nRAStas    0RARUs    1RARUs    2RARUs    4RARUs    8RARUs   16RARUs\n");
	
	setApAntennas(1);
	
	int step = 1;
	for (int nStas = 0; nStas <= 100; nStas += step) {
		if (nStas == 20) step = 10;

		printf("%7d    ", nStas);
		
		//case of nRARUs=0 is treated separately from the following "for" loop because 0 *= 2 is always 0 and causes an infinit loop
		result = simulate_wlan(BANDWIDTH_80MHz, PURE_UL_OFDMA, RU_SIZE_52_TONES, 0, nStas, MCS_6, 100);
		printf("%6.2fM   ", result.throughput);

		for (int nRARUs = 1; nRARUs <= 16; nRARUs *= 2) {
			result = simulate_wlan(BANDWIDTH_80MHz, PURE_UL_OFDMA, RU_SIZE_52_TONES, nRARUs, nStas, MCS_6, 100);
			printf("%6.2fM   ", result.throughput);
		}
		
		printf("\n");
	}
}


/*
This function measures the average transmission delays of pure UL OFDMA as a function of the number of contending stations and the number of RA RUs.
It is used to build Figure 8 of the paper
*/
void pureOfdmaDelays() {
	struct wlan_result result;
	
	printf("Delays (us) of pure UL OFDMA (1x1 UL MU-MIMO):\n");
	printf("we measure the average delay as a function of the number of contending stations (not all stations)\n");
	printf("nRAStas    1RARUs    2RARUs    4RARUs    8RARUs   16RARUs\n");
	
	setApAntennas(1);
	
	int step = 1;
	for (int nStas = 0; nStas <= 100; nStas += step) {
		if (nStas == 20) step = 10;

		printf("%7d  ", nStas);


		for (int nRARUs = 1; nRARUs <= 16; nRARUs *= 2) {
			result = simulate_wlan(BANDWIDTH_80MHz, PURE_UL_OFDMA, RU_SIZE_52_TONES, nRARUs, nStas, MCS_6, 100);
			printf("%8d  ", result.avgRAStasTxDelaysMicros);
		}
		
		printf("\n");
	}
}


/*
This function sets the number of stations to 16 (number of scheduled + unscheduled stations = 16) and compares 
the throughput of legacy full bandwidth transmissions (i.e. pure EDCA), "UL OFDMA with EDCA" and pure UL OFDMA 
as a function of the number of of RA RUs.
It is used to build Figure 9 of the paper
*/
void throughputComparison1() {
	struct wlan_result result;
	
	printf("Throughput of EDCA, UL OFDMA with EDCA, and pure UL OFDMA (max A-MPDU length = 10 MPDUs):\n");
	printf("We vary the number of RA RUs and keep the same number of stations (16 STAs)\n");
	printf("nRARUs      pureEDCA  ULOFDMAwEDCA   pureULOFDMA\n");
	
	setApAntennas(1);
	
	for (int nRARUs = 0; nRARUs <= 16; nRARUs++) {
		printf("%6d  ", nRARUs);
		result = simulate_wlan(BANDWIDTH_80MHz, PURE_EDCA, RU_SIZE_52_TONES, 0, 16, MCS_6, 10);
		printf("%12.2fM ", result.throughput);

		result = simulate_wlan(BANDWIDTH_80MHz, UL_OFDMA_WITH_EDCA, RU_SIZE_52_TONES, nRARUs, nRARUs, MCS_6, 10); 
		printf("%12.2fM ", result.throughput);

		result = simulate_wlan(BANDWIDTH_80MHz, PURE_UL_OFDMA, RU_SIZE_52_TONES, nRARUs, nRARUs, MCS_6, 10);
		printf("%12.2fM ", result.throughput);

		printf("\n");
	}
}


/*
This function compares the throughput of legacy full bandwidth transmissions (i.e. pure EDCA), 
"UL OFDMA with EDCA" and pure UL OFDMA as a function of the number of contending stations.
It is used to build Figure 10 of the paper
*/
void ulThroughputComparison2() {
	struct wlan_result result;
	
	printf("Throughput of EDCA, UL OFDMA with EDCA, and pure UL OFDMA (max A-MPDU length = 10 MPDUs):\n");
	printf("We vary the number of contending stations\n");
	printf("nRAStas              pureEDCA    ULOFDMAwEDCA_0RARU  ULOFDMAwEDCA_allRARU     pureULOFDMA_0RARU   pureULOFDMA_allRARU\n");
	
	setApAntennas(1);
	
	int step = 1;
	for (int nStas = 0; nStas <= 100; nStas += step) {
		if (nStas == 20) step = 10;
		
		printf("%7d  ", nStas);
		result = simulate_wlan(BANDWIDTH_80MHz, PURE_EDCA, RU_SIZE_52_TONES, 0, nStas, MCS_6, 10);
		printf("%20.2fM ", result.throughput);

		result = simulate_wlan(BANDWIDTH_80MHz, UL_OFDMA_WITH_EDCA, RU_SIZE_52_TONES, 0, nStas, MCS_6, 10); 
		printf("%20.2fM ", result.throughput);

		//all RUs are RA RUs
		result = simulate_wlan(BANDWIDTH_80MHz, UL_OFDMA_WITH_EDCA, RU_SIZE_52_TONES, 16, nStas, MCS_6, 10);
		printf("%20.2fM ", result.throughput);

		result = simulate_wlan(BANDWIDTH_80MHz, PURE_UL_OFDMA, RU_SIZE_52_TONES, 0, nStas, MCS_6, 10);
		printf("%20.2fM ", result.throughput);

		//all RUs are RA RUs
		result = simulate_wlan(BANDWIDTH_80MHz, PURE_UL_OFDMA, RU_SIZE_52_TONES, 16, nStas, MCS_6, 10);
		printf("%20.2fM ", result.throughput);

		printf("\n");
	}
}


/*
This function compares the average transmission delays of legacy full bandwidth transmissions (i.e. pure EDCA), 
"UL OFDMA with EDCA" and pure UL OFDMA as a function of the number of contending stations.
It is used to build Figure 11 of the paper
*/
void ulDelayComparison() {
	struct wlan_result result;
	
	printf("Delays (us) of EDCA, UL OFDMA with EDCA, and pure UL OFDMA (max A-MPDU length = 10 MPDUs):\n");
	printf("We vary the number of contending stations\n");
	printf("nRAStas              pureEDCA    ULOFDMAwEDCA_0RARU  ULOFDMAwEDCA_allRARU   pureULOFDMA_allRARU\n");
	
	setApAntennas(1);
	
	int step = 1;
	for (int nStas = 0; nStas <= 100; nStas += step) {
		if (nStas == 20) step = 10;
		
		printf("%7d  ", nStas);
		result = simulate_wlan(BANDWIDTH_80MHz, PURE_EDCA, RU_SIZE_52_TONES, 0, nStas, MCS_6, 10);
		printf("%20d  ", result.avgRAStasTxDelaysMicros);

		result = simulate_wlan(BANDWIDTH_80MHz, UL_OFDMA_WITH_EDCA, RU_SIZE_52_TONES, 0, nStas, MCS_6, 10); 
		printf("%20d  ", result.avgRAStasTxDelaysMicros);

		//all RUs are RA RUs
		result = simulate_wlan(BANDWIDTH_80MHz, UL_OFDMA_WITH_EDCA, RU_SIZE_52_TONES, 16, nStas, MCS_6, 10);
		printf("%20d  ", result.avgRAStasTxDelaysMicros);

		//all RUs are RA RUs
		result = simulate_wlan(BANDWIDTH_80MHz, PURE_UL_OFDMA, RU_SIZE_52_TONES, 16, nStas, MCS_6, 10);
		printf("%20d  ", result.avgRAStasTxDelaysMicros);

		printf("\n");
	}
}


/*
This function compares the throughput of legacy full bandwidth transmissions (i.e. pure EDCA), "UL OFDMA with EDCA" 
and pure UL OFDMA as a function of the maximum A-MPDU length (i.e. number of aggregated MPDUs within an A-MPDU).
It is used to build Figure 12 of the paper
*/
void ulThroughputComparison3() {
	struct wlan_result result;
	
	printf("Throughput of EDCA, UL OFDMA with EDCA, and pure UL OFDMA (nStas=16):\n");
	printf("We vary the A-MPDU length up to max PPDU duration\n");
	printf("A-MPDU_len              pureEDCA    ULOFDMAwEDCA_0RARU  ULOFDMAwEDCA_allRARU     pureULOFDMA_0RARU   pureULOFDMA_allRARU\n");
	
	setApAntennas(1);
	
	const int nStas = 16;
	int step = 1;
	for (int ampdu_len = 1; ampdu_len <= 180; ampdu_len += step) {
		if (ampdu_len == 10) step = 10;
		
		printf("%10d  ", ampdu_len);
		result = simulate_wlan(BANDWIDTH_80MHz, PURE_EDCA, RU_SIZE_52_TONES, 0, nStas, MCS_6, ampdu_len);
		printf("%20.2fM ", result.throughput);

		result = simulate_wlan(BANDWIDTH_80MHz, UL_OFDMA_WITH_EDCA, RU_SIZE_52_TONES, 0, 0, MCS_6, ampdu_len); 
		printf("%20.2fM ", result.throughput);

		result = simulate_wlan(BANDWIDTH_80MHz, UL_OFDMA_WITH_EDCA, RU_SIZE_52_TONES, 16, nStas, MCS_6, ampdu_len);
		printf("%20.2fM ", result.throughput);

		result = simulate_wlan(BANDWIDTH_80MHz, PURE_UL_OFDMA, RU_SIZE_52_TONES, 0, 0, MCS_6, ampdu_len);
		printf("%20.2fM ", result.throughput);

		result = simulate_wlan(BANDWIDTH_80MHz, PURE_UL_OFDMA, RU_SIZE_52_TONES, 16, nStas, MCS_6, ampdu_len);
		printf("%20.2fM ", result.throughput);

		printf("\n");
	}
}


/*
This function compares the throughput of legacy full bandwidth transmissions (i.e. pure EDCA) and DL OFDMA 
as a function of the maximum A-MPDU length (i.e. number of aggregated MPDUs within an A-MPDU).
It is used to build Figure 13 of the paper
*/
void dlOfdmaThroughput() {
	struct wlan_result result;
	
	printf("Throughput of EDCA and DL OFDMA with EDCA:\n");
	printf("We vary the A-MPDU length up to max PPDU duration\n");
	printf("A-MPDU_len              pureEDCA    DLOFDMAwEDCA\n");
	
	setApAntennas(1);
	
	const int nStas = 1;
	int step = 1;
	for (int ampdu_len = 1; ampdu_len <= 180; ampdu_len += step) {
		if (ampdu_len == 10) step = 10;
		
		printf("%10d  ", ampdu_len);
		result = simulate_wlan(BANDWIDTH_80MHz, PURE_EDCA, RU_SIZE_52_TONES, 0, nStas, MCS_6, ampdu_len);
		printf("%20.2fM ", result.throughput);

		result = simulate_wlan(BANDWIDTH_80MHz, DL_OFDMA_WITH_EDCA, RU_SIZE_52_TONES, 0, 0, MCS_6, ampdu_len); 
		printf("%20.2fM ", result.throughput);

		printf("\n");
	}
}


/*
This function compares the throughput of legacy full bandwidth transmissions (i.e. pure EDCA) and UL MU-MIMO
as a function of the maximum A-MPDU length and the number of spatial streams.
It is used to build Figure 14 of the paper
*/
void ulMuMimoThroughput1() {
	struct wlan_result result;
	
	printf("Throughput of EDCA and UL MU-MIMO with EDCA:\n");
	printf("We vary the A-MPDU length up to max PPDU duration\n");
	printf("A-MPDU_len          pureEDCA     UL_MU-MIMO_2S      UL_MU-MIMO_4S      UL_MU-MIMO_6S     UL_MU-MIMO_8S\n");
	
	setApAntennas(1);
	
	const int nStas = 1;
	int step = 1;
	for (int ampdu_len = 1; ampdu_len <= 180; ampdu_len += step) {
		if (ampdu_len == 10) step = 10;
		
		printf("%10d  ", ampdu_len);
		result = simulate_wlan(BANDWIDTH_80MHz, PURE_EDCA, RU_SIZE_996_TONES, 0, nStas, MCS_6, ampdu_len);
		printf("%16.2fM ", result.throughput);

		setApAntennas(2);
		result = simulate_wlan(BANDWIDTH_80MHz, UL_OFDMA_WITH_EDCA, RU_SIZE_996_TONES, 0, 0, MCS_6, ampdu_len); 
		printf("%16.2fM ", result.throughput);

		setApAntennas(4);
		result = simulate_wlan(BANDWIDTH_80MHz, UL_OFDMA_WITH_EDCA, RU_SIZE_996_TONES, 0, 0, MCS_6, ampdu_len); 
		printf("%16.2fM ", result.throughput);

		setApAntennas(6);
		result = simulate_wlan(BANDWIDTH_80MHz, UL_OFDMA_WITH_EDCA, RU_SIZE_996_TONES, 0, 0, MCS_6, ampdu_len); 
		printf("%16.2fM ", result.throughput);

		setApAntennas(8);
		result = simulate_wlan(BANDWIDTH_80MHz, UL_OFDMA_WITH_EDCA, RU_SIZE_996_TONES, 0, 0, MCS_6, ampdu_len); 
		printf("%16.2fM ", result.throughput);

		printf("\n");
	}
}


/*
This function compares the throughput of legacy full bandwidth transmissions (i.e. pure EDCA) and UL MU-MIMO
as a function of the maximum A-MPDU length and the RU size.
It is used to build Figure 15 of the paper
*/
void ulMuMimoThroughput2() {
	struct wlan_result result;
	
	printf("Throughput of EDCA and UL MU-MIMO with EDCA (different numbers of RUs):\n");
	printf("We vary the A-MPDU length up to max PPDU duration\n");
	printf("A-MPDU_len          pureEDCA     UL_MU-MIMO106     UL_MU-MIMO242     UL_MU-MIMO484     UL_MU-MIMO996\n");
	
	setApAntennas(4);
	
	const int nStas = 1;
	int step = 1;
	for (int ampdu_len = 1; ampdu_len <= 180; ampdu_len += step) {
		if (ampdu_len == 10) step = 10;
		
		printf("%10d  ", ampdu_len);
		result = simulate_wlan(BANDWIDTH_80MHz, PURE_EDCA, RU_SIZE_106_TONES, 0, nStas, MCS_6, ampdu_len);
		printf("%16.2fM ", result.throughput);

		result = simulate_wlan(BANDWIDTH_80MHz, UL_OFDMA_WITH_EDCA, RU_SIZE_106_TONES, 0, 0, MCS_6, ampdu_len); 
		printf("%16.2fM ", result.throughput);

		result = simulate_wlan(BANDWIDTH_80MHz, UL_OFDMA_WITH_EDCA, RU_SIZE_242_TONES, 0, 0, MCS_6, ampdu_len); 
		printf("%16.2fM ", result.throughput);

		result = simulate_wlan(BANDWIDTH_80MHz, UL_OFDMA_WITH_EDCA, RU_SIZE_484_TONES, 0, 0, MCS_6, ampdu_len); 
		printf("%16.2fM ", result.throughput);

		result = simulate_wlan(BANDWIDTH_80MHz, UL_OFDMA_WITH_EDCA, RU_SIZE_996_TONES, 0, 0, MCS_6, ampdu_len); 
		printf("%16.2fM ", result.throughput);

		printf("\n");
	}
}


/*
This function compares the throughput of legacy full bandwidth transmissions (i.e. pure EDCA) and UL MU-MIMO
as a function of the number of contending stations and the RU size.
It is used to build Figure 16 of the paper
*/
void ulMuMimoThroughput3() {
	struct wlan_result result;
	
	printf("Throughput of EDCA and UL MU-MIMO with EDCA (max A-MPDU length = 10 MPDUs):\n");
	printf("We vary the number of contending stations\n");
	printf("nRAStas          pureEDCA     UL_MU-MIMO106     UL_MU-MIMO242     UL_MU-MIMO484     UL_MU-MIMO996\n");
	
	setApAntennas(4);
	
	int ampdu_len = 10;
	int nRARUs = 0;

	int step = 1;
	for (int nStas = 0; nStas <= 100; nStas += step) {
		if (nStas == 10) step = 10;
		
		printf("%7d  ", nStas);
		result = simulate_wlan(BANDWIDTH_80MHz, PURE_EDCA, RU_SIZE_106_TONES, nRARUs, nStas, MCS_6, ampdu_len);
		printf("%16.2fM ", result.throughput);

		result = simulate_wlan(BANDWIDTH_80MHz, UL_OFDMA_WITH_EDCA, RU_SIZE_106_TONES, nRARUs, nStas, MCS_6, ampdu_len); 
		printf("%16.2fM ", result.throughput);

		result = simulate_wlan(BANDWIDTH_80MHz, UL_OFDMA_WITH_EDCA, RU_SIZE_242_TONES, nRARUs, nStas, MCS_6, ampdu_len); 
		printf("%16.2fM ", result.throughput);

		result = simulate_wlan(BANDWIDTH_80MHz, UL_OFDMA_WITH_EDCA, RU_SIZE_484_TONES, nRARUs, nStas, MCS_6, ampdu_len); 
		printf("%16.2fM ", result.throughput);

		result = simulate_wlan(BANDWIDTH_80MHz, UL_OFDMA_WITH_EDCA, RU_SIZE_996_TONES, nRARUs, nStas, MCS_6, ampdu_len);
		printf("%16.2fM ", result.throughput);

		printf("\n");
	}
}


/*
This function compares the throughput of legacy full bandwidth transmissions (i.e. pure EDCA) and DL MU-MIMO
as a function of the sounding period and the RU size.
It is used to build Figure 17 of the paper
*/
void dlMuMimoThroughput1() {
	struct wlan_result result;
	
	printf("Throughput of EDCA and DL MU-MIMO with EDCA (A-MPDU length = 10 MPDUs):\n");
	printf("We vary the sounding period\n");
	printf("Snd_period          pureEDCA     DL_MU-MIMO106     DL_MU-MIMO242     DL_MU-MIMO484     DL_MU-MIMO996\n");
	
	setApAntennas(4); //only supported value is 4 antennas
	setNBeamformees(1); //nBfees is required to count the sounding sequences. nBfees=1 means 1 sequence 
	
	const int nStas = 1;
	int ampdu_len = 10;
	
	int step = 1;
	for (int snd = 1; snd <= 100; snd += step) {
		if (snd == 10) step = 10;
		
		printf("%10d  ", snd);
		setSoundingPeriodMillis(snd);
		
		result = simulate_wlan(BANDWIDTH_80MHz, PURE_EDCA, RU_SIZE_106_TONES, 0, nStas, MCS_6, ampdu_len);
		printf("%16.2fM ", result.throughput);

		result = simulate_wlan(BANDWIDTH_80MHz, DL_MU_MIMO_WITH_EDCA, RU_SIZE_106_TONES, 0, 0, MCS_6, ampdu_len); 
		printf("%16.2fM ", result.throughput);

		result = simulate_wlan(BANDWIDTH_80MHz, DL_MU_MIMO_WITH_EDCA, RU_SIZE_242_TONES, 0, 0, MCS_6, ampdu_len); 
		printf("%16.2fM ", result.throughput);

		result = simulate_wlan(BANDWIDTH_80MHz, DL_MU_MIMO_WITH_EDCA, RU_SIZE_484_TONES, 0, 0, MCS_6, ampdu_len); 
		printf("%16.2fM ", result.throughput);

		result = simulate_wlan(BANDWIDTH_80MHz, DL_MU_MIMO_WITH_EDCA, RU_SIZE_996_TONES, 0, 0, MCS_6, ampdu_len); 
		printf("%16.2fM ", result.throughput);

		printf("\n");
	}
}


/*
This function compares the average transmission delays of legacy full bandwidth transmissions (i.e. pure EDCA) and DL MU-MIMO
as a function of the sounding period and the RU size.
It is used to build Figure 18 of the paper
*/
void dlMuMimoDelays1() {
	struct wlan_result result;
	
	printf("Delays (us) of EDCA and DL MU-MIMO with EDCA (A-MPDU length = 10 MPDUs):\n");
	printf("We vary the sounding period\n");
	printf("Snd_period          pureEDCA     DL_MU-MIMO106     DL_MU-MIMO242     DL_MU-MIMO484     DL_MU-MIMO996\n");
	
	setApAntennas(4); //only supported value is 4 antennas
	setNBeamformees(1); //nBfees is required to count the sounding sequences. nBfees=1 means 1 sequence 
	
	const int nStas = 1;
	int ampdu_len = 10;
	
	int step = 1;
	for (int snd = 1; snd <= 100; snd += step) {
		if (snd == 10) step = 10;
		
		printf("%10d  ", snd);
		setSoundingPeriodMillis(snd);
		
		//in case of PURE_EDCA, the AP does not contend so we set 1 contending STA and we record its delays
		result = simulate_wlan(BANDWIDTH_80MHz, PURE_EDCA, RU_SIZE_106_TONES, 0, nStas, MCS_6, ampdu_len);
		printf("%16d  ", result.avgRAStasTxDelaysMicros);
		
		result = simulate_wlan(BANDWIDTH_80MHz, DL_MU_MIMO_WITH_EDCA, RU_SIZE_106_TONES, 0, 0, MCS_6, ampdu_len); 
		printf("%16d  ", result.avgApTxDelaysMicros);

		result = simulate_wlan(BANDWIDTH_80MHz, DL_MU_MIMO_WITH_EDCA, RU_SIZE_242_TONES, 0, 0, MCS_6, ampdu_len); 
		printf("%16d  ", result.avgApTxDelaysMicros);

		result = simulate_wlan(BANDWIDTH_80MHz, DL_MU_MIMO_WITH_EDCA, RU_SIZE_484_TONES, 0, 0, MCS_6, ampdu_len); 
		printf("%16d  ", result.avgApTxDelaysMicros);

		result = simulate_wlan(BANDWIDTH_80MHz, DL_MU_MIMO_WITH_EDCA, RU_SIZE_996_TONES, 0, 0, MCS_6, ampdu_len); 
		printf("%16d  ", result.avgApTxDelaysMicros);

		printf("\n");
	}
}


/*
This function compares the throughput of legacy full bandwidth transmissions (i.e. pure EDCA) and DL MU-MIMO
as a function of the number of beamformees and the RU size.
It is used to build Figure 19 of the paper
*/
void dlMuMimoThroughput2() {
	struct wlan_result result;
	
	printf("Throughput of EDCA and DL MU-MIMO with EDCA (sounding period of 10ms):\n");
	printf("We vary the number of beamformees\n");
	printf("Bfees_cnt          pureEDCA     DL_MU-MIMO106     DL_MU-MIMO242     DL_MU-MIMO484     DL_MU-MIMO996\n");
	
	setApAntennas(4); //only supported value is 4 antennas
	setSoundingPeriodMillis(10);
	
	const int nStas = 1;
	int ampdu_len = 10;
	
	int step = 1;
	for (int nBfees = 1; nBfees <= 200; nBfees += step) {
		if (nBfees == 10) step = 6; //10 then 16
		else if (nBfees == 16) step = 4; //16 then 20
		else if (nBfees == 20) step = 10; //20 then 30
		else if (nBfees == 30) step = 2; //30 then 32
		else if (nBfees == 32) step = 8; //32 then 40
		else if (nBfees == 40) step = 10; //40, 50, 60, ...
		
		printf("%9d  ", nBfees);
		setNBeamformees(nBfees);
		
		result = simulate_wlan(BANDWIDTH_80MHz, PURE_EDCA, RU_SIZE_106_TONES, 0, nStas, MCS_6, ampdu_len);
		printf("%16.2fM ", result.throughput);

		result = simulate_wlan(BANDWIDTH_80MHz, DL_MU_MIMO_WITH_EDCA, RU_SIZE_106_TONES, 0, 0, MCS_6, ampdu_len); 
		printf("%16.2fM ", result.throughput);

		result = simulate_wlan(BANDWIDTH_80MHz, DL_MU_MIMO_WITH_EDCA, RU_SIZE_242_TONES, 0, 0, MCS_6, ampdu_len); 
		printf("%16.2fM ", result.throughput);

		result = simulate_wlan(BANDWIDTH_80MHz, DL_MU_MIMO_WITH_EDCA, RU_SIZE_484_TONES, 0, 0, MCS_6, ampdu_len); 
		printf("%16.2fM ", result.throughput);

		result = simulate_wlan(BANDWIDTH_80MHz, DL_MU_MIMO_WITH_EDCA, RU_SIZE_996_TONES, 0, 0, MCS_6, ampdu_len); 
		printf("%16.2fM ", result.throughput);

		printf("\n");
	}
}


/*
This function compares the throughput of legacy full bandwidth transmissions (i.e. pure EDCA) and DL MU-MIMO
as a function of the maximum A-MPDU length and the RU size.
It is used to build Figure 20 of the paper
*/
void dlMuMimoThroughput3() {
	struct wlan_result result;
	
	printf("Throughput of EDCA and DL MU-MIMO with EDCA (sounding period of 10ms):\n");
	printf("We vary the A-MPDU length up to max PPDU duration\n");
	printf("A-MPDU_len          pureEDCA     DL_MU-MIMO106     DL_MU-MIMO242     DL_MU-MIMO484     DL_MU-MIMO996\n");
	
	setApAntennas(4); //only supported value is 4 antennas
	setSoundingPeriodMillis(10);
	setNBeamformees(1); //nBfees is required to count the sounding sequences. nBfees=1 means 1 sequence 
	
	const int nStas = 1;
	int step = 1;
	for (int ampdu_len = 1; ampdu_len <= 180; ampdu_len += step) {
		if (ampdu_len == 10) step = 10;
		
		printf("%10d  ", ampdu_len);
		
		result = simulate_wlan(BANDWIDTH_80MHz, PURE_EDCA, RU_SIZE_106_TONES, 0, nStas, MCS_6, ampdu_len);
		printf("%16.2fM ", result.throughput);

		result = simulate_wlan(BANDWIDTH_80MHz, DL_MU_MIMO_WITH_EDCA, RU_SIZE_106_TONES, 0, 0, MCS_6, ampdu_len); 
		printf("%16.2fM ", result.throughput);

		result = simulate_wlan(BANDWIDTH_80MHz, DL_MU_MIMO_WITH_EDCA, RU_SIZE_242_TONES, 0, 0, MCS_6, ampdu_len); 
		printf("%16.2fM ", result.throughput);

		result = simulate_wlan(BANDWIDTH_80MHz, DL_MU_MIMO_WITH_EDCA, RU_SIZE_484_TONES, 0, 0, MCS_6, ampdu_len); 
		printf("%16.2fM ", result.throughput);

		result = simulate_wlan(BANDWIDTH_80MHz, DL_MU_MIMO_WITH_EDCA, RU_SIZE_996_TONES, 0, 0, MCS_6, ampdu_len); 
		printf("%16.2fM ", result.throughput);

		printf("\n");
	}
}


/*
This function compares the throughput of legacy full bandwidth transmissions (i.e. pure EDCA) and DL MU-MIMO
as a function of the sounding period and the RU size.
The results of this function are not included in the paper
*/
void dlMuMimoDelays2() {
	struct wlan_result result;
	
	printf("Delays (us) of EDCA and DL MU-MIMO with EDCA (sounding period of 10ms):\n");
	printf("We vary the number of beamformees\n");
	printf("Bfees_cnt          pureEDCA     DL_MU-MIMO106     DL_MU-MIMO242     DL_MU-MIMO484     DL_MU-MIMO996\n");
	
	setApAntennas(4); //only supported value is 4 antennas
	setSoundingPeriodMillis(10);
	
	const int nStas = 1;
	int ampdu_len = 10;
	
	int step = 1;
	for (int nBfees = 1; nBfees <= 200; nBfees += step) {
		if (nBfees == 10) step = 6; //10 then 16
		else if (nBfees == 16) step = 4; //16 then 20
		else if (nBfees == 20) step = 10; //20 then 30
		else if (nBfees == 30) step = 2; //30 then 32
		else if (nBfees == 32) step = 8; //32 then 40
		else if (nBfees == 40) step = 10; //40, 50, 60, ...
		
		printf("%9d  ", nBfees);
		setNBeamformees(nBfees);
		
		//in case of PURE_EDCA, the AP does not contend so we set 1 contending STA and we record its delays
		result = simulate_wlan(BANDWIDTH_80MHz, PURE_EDCA, RU_SIZE_106_TONES, 0, nStas, MCS_6, ampdu_len);
		printf("%16d  ", result.avgRAStasTxDelaysMicros);
		
		result = simulate_wlan(BANDWIDTH_80MHz, DL_MU_MIMO_WITH_EDCA, RU_SIZE_106_TONES, 0, 0, MCS_6, ampdu_len); 
		printf("%16d  ", result.avgApTxDelaysMicros);

		result = simulate_wlan(BANDWIDTH_80MHz, DL_MU_MIMO_WITH_EDCA, RU_SIZE_242_TONES, 0, 0, MCS_6, ampdu_len); 
		printf("%16d  ", result.avgApTxDelaysMicros);

		result = simulate_wlan(BANDWIDTH_80MHz, DL_MU_MIMO_WITH_EDCA, RU_SIZE_484_TONES, 0, 0, MCS_6, ampdu_len); 
		printf("%16d  ", result.avgApTxDelaysMicros);

		result = simulate_wlan(BANDWIDTH_80MHz, DL_MU_MIMO_WITH_EDCA, RU_SIZE_996_TONES, 0, 0, MCS_6, ampdu_len); 
		printf("%16d  ", result.avgApTxDelaysMicros);

		printf("\n");
	}
}


int main(int argc, char** argv) {
	pureOfdmaCollisionRate();
	printf("\n");
	pureOfdmaThroughput();
	printf("\n");
	pureOfdmaDelays();
	printf("\n");
	throughputComparison1();
	printf("\n");
	ulThroughputComparison2();
	printf("\n");
	ulDelayComparison();
	printf("\n");
	ulThroughputComparison3();
	printf("\n");
	dlOfdmaThroughput();
	printf("\n");
	ulMuMimoThroughput1();
	printf("\n");
	ulMuMimoThroughput2();
	printf("\n");
	ulMuMimoThroughput3();
	printf("\n");
	dlMuMimoThroughput1();
	printf("\n");
	dlMuMimoDelays1();
	printf("\n");
	dlMuMimoThroughput2();
	printf("\n");
	dlMuMimoThroughput3();
	printf("\n");
	dlMuMimoDelays2();
	printf("\n");
	
	
	return 0;
}

