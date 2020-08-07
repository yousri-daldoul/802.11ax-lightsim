#include <stdio.h>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <algorithm>    // std::min
#include <math.h>       /* ceil */
#include "he_wlan.h"

/* run this program using the console pauser or add your own getch, system("pause") or input loop */

#define CW_MIN                   15
#define CW_MAX                   255
#define OCW_MIN                  7
#define OCW_MAX                  31
#define AIFS                     34
#define SIFS                     16
#define SLOT_TIME                9
#define MAX_PPDU_DURATION_US     5484 //5484µs
#define ETHERNET_FRAME_LENGTH    (1500*8) //length in bits
#define MAX_SIMULATION_TIME_US   1000000000 //1000 sec


int m_nApAntennas = 1;
void setApAntennas(int nApAntennas) {
	m_nApAntennas = nApAntennas;
}


int m_soundingPeriodMicros = 100 * 1000; //100ms
void setSoundingPeriodMillis(int soundingPeriodMillis) {
	m_soundingPeriodMicros = soundingPeriodMillis * 1000;
}


int m_nBfees = 0; //invalid value
void setNBeamformees(int nBfees) {
	m_nBfees = nBfees;
}

int bw2ru_size(int bw) {
	//convert bw to ru_size;
	int ru_size = 0;
	
	if (bw == BANDWIDTH_20MHz) ru_size = RU_SIZE_242_TONES;
	else if (bw == BANDWIDTH_40MHz) ru_size = RU_SIZE_484_TONES;
	else if (bw == BANDWIDTH_80MHz) ru_size = RU_SIZE_996_TONES;
	else if (bw == BANDWIDTH_160MHz) ru_size = RU_SIZE_2x996_TONES;
	
	return ru_size;
}

//return the maximum supported Ressource Units (RU) as a function of the channel width
//return 0 if either the channel width or the RU size is unsupported
int getMaxRUsPerChannelWidth(int bw, int ru_size) {
	int maxRUs = 0;
	if (bw == BANDWIDTH_20MHz) {
		if (ru_size == RU_SIZE_26_TONES) maxRUs = 9;
		else if (ru_size == RU_SIZE_52_TONES) maxRUs = 4;
		else if (ru_size == RU_SIZE_106_TONES) maxRUs = 2;
		else if (ru_size == RU_SIZE_242_TONES) maxRUs = 1;
		//else unsupported ru_size
	}
	else if (bw == BANDWIDTH_40MHz) {
		if (ru_size == RU_SIZE_26_TONES) maxRUs = 18;
		else if (ru_size == RU_SIZE_52_TONES) maxRUs = 8;
		else if (ru_size == RU_SIZE_106_TONES) maxRUs = 4;
		else if (ru_size == RU_SIZE_242_TONES) maxRUs = 2;
		else if (ru_size == RU_SIZE_484_TONES) maxRUs = 1;
		//else unsupported ru_size
	}
	else if (bw == BANDWIDTH_80MHz) {
		if (ru_size == RU_SIZE_26_TONES) maxRUs = 37;
		else if (ru_size == RU_SIZE_52_TONES) maxRUs = 16;
		else if (ru_size == RU_SIZE_106_TONES) maxRUs = 8;
		else if (ru_size == RU_SIZE_242_TONES) maxRUs = 4;
		else if (ru_size == RU_SIZE_484_TONES) maxRUs = 2;
		else if (ru_size == RU_SIZE_996_TONES) maxRUs = 1;
		//else unsupported ru_size
	}
	else if (bw == BANDWIDTH_160MHz) {
		if (ru_size == RU_SIZE_26_TONES) maxRUs = 74;
		else if (ru_size == RU_SIZE_52_TONES) maxRUs = 32;
		else if (ru_size == RU_SIZE_106_TONES) maxRUs = 16;
		else if (ru_size == RU_SIZE_242_TONES) maxRUs = 8;
		else if (ru_size == RU_SIZE_484_TONES) maxRUs = 4;
		else if (ru_size == RU_SIZE_996_TONES) maxRUs = 2;
		else if (ru_size == RU_SIZE_2x996_TONES) maxRUs = 1;
		//else unsupported ru_size
	}

	return maxRUs;
}

//returns the maximum number of MPDUs within an A-MPDU
int getOfdmaAMpduLength(int mcs, int ru_size, int max_ppdu_duration, int mpdulen) {
	//mcs6 80MHz (RU996) : 324.3Mbps
	//mcs6 RU52 : 15.9 Mbps
	double data_rate;
	int cnt = 0;
	
	if (mcs != 6) {
		printf("only MCS 6 is supported\n");
		exit(0);
	}
	
	if (ru_size == RU_SIZE_26_TONES) {
		data_rate = 7.9;
	}
	else if (ru_size == RU_SIZE_52_TONES) {
		data_rate = 15.9;
	}
	else if (ru_size == RU_SIZE_106_TONES) {
		data_rate = 33.8;
	}
	else if (ru_size == RU_SIZE_242_TONES) {
		data_rate = 77.4;
	}
	else if (ru_size == RU_SIZE_484_TONES) {
		data_rate = 154.9;
	}
	else if (ru_size == RU_SIZE_996_TONES) {
		data_rate = 324.3;
	}
	else if (ru_size == RU_SIZE_2x996_TONES) {
		data_rate = 648.5;
	}
	else {
		printf("RU-%d is not supported !\n", ru_size);
		exit(0);
	}
	
	cnt = data_rate * (max_ppdu_duration - 40) / (44 * 8 + mpdulen); // 40 is the PHY header, and 44 is the MAC header + MPDU delimiter
	return cnt;
}

int getEdcaAMpduLength(int mcs, int bw, int max_ppdu_duration, int mpdulen) {
	int ru_size = bw2ru_size(bw);
	
	return getOfdmaAMpduLength(mcs, ru_size, max_ppdu_duration, mpdulen);
}

int getPpduDuration(int mcs, int ru_size, int psduLen) {
	int ppdu_duration = 0;
	int Ndbps = 0;
	if (mcs != 6) {
		printf("only MCS 6 is supported\n");
		exit(0);
	}
	
	if (ru_size == RU_SIZE_26_TONES) {
		Ndbps = 108;
	}
	else if (ru_size == RU_SIZE_52_TONES) {
		Ndbps = 216; //RU-52, MCS6, 1SS (corresponds to a data rate of 15.9Mbps)
	}
	else if (ru_size == RU_SIZE_106_TONES) {
		Ndbps = 459;
	}
	else if (ru_size == RU_SIZE_242_TONES) {
		Ndbps = 1053;
	}
	else if (ru_size == RU_SIZE_484_TONES) {
		Ndbps = 2106;
	}
	else if (ru_size == RU_SIZE_996_TONES) {
		Ndbps = 4410; //RU-996 (80MHz), MCS6, 1SS (corresponds to a data rate of 324.3Mbps)
	}
	else if (ru_size == RU_SIZE_2x996_TONES) {
		Ndbps = 8820;
	}
	else {
		printf("RU-%d is not supported !\n", ru_size);
		exit(0);
	}
	
	ppdu_duration = 40 + ceil((16 + psduLen + 6.0)/Ndbps) * (12.8 + 0.8);
	return ppdu_duration;
}

int getOfdmaAMpduDuration(int mcs, int ru_size, int ampdu_len, int mpdulen) {
	int psduLen = (44 * 8 + mpdulen) * ampdu_len;
	return getPpduDuration(mcs, ru_size, psduLen);
}

int getEdcaAMpduDuration(int mcs, int bw, int ampdu_len, int mpdulen) {
	int ru_size = bw2ru_size(bw);
	int psduLen = (44 * 8 + mpdulen) * ampdu_len;
	return getPpduDuration(mcs, ru_size, psduLen);
}

int getSoundingDuration(int bw, int mcs, int ru_size, int nBfees) {
	//for m_nApAntennas streams in DL we can receiver m_nApAntennas in UL => we need a single BRP/CompressedBeamforming sequence
	//ndpa + SIFS + ndp + SIFS + brp + SIFS + cbr
	//ndpa and brp are transmitted on the entire channel width
	if (m_nApAntennas != 4) {
		//this case should not accur
		printf("Only 4x4 DL MU-MIMO is supported\nExit ...\n");
		exit(0);
	}
	
	if (nBfees < 1) {
		//this case should not accur
		printf("invalid nStas=%d\nExit ...\n", nBfees);
		exit(0);
	}
	
	int cbr_ebr_fields = 0; //HE Compressed Beamforming Report and HE MU Exclusive Beamforming Report fields
	if (ru_size == RU_SIZE_106_TONES) cbr_ebr_fields = 253 + 14;
	else if (ru_size == RU_SIZE_242_TONES) cbr_ebr_fields = 577 + 32;
	else if (ru_size == RU_SIZE_484_TONES) cbr_ebr_fields = 1099 + 61;
	else if (ru_size == RU_SIZE_996_TONES) cbr_ebr_fields = 2251 + 125;
	else if (ru_size == RU_SIZE_2x996_TONES) cbr_ebr_fields = 4501 + 250;
	else {
		//this case should not accur
		printf("ru_size is not supported with DL MU-MIMO\nExit ...\n");
		exit(0);
	}

	int ndpa = getPpduDuration(mcs, bw2ru_size(bw), (21 + 2 * m_nApAntennas) * 8);
	int ndp = 40;
	int brp = getPpduDuration(mcs, bw2ru_size(bw), (28 + 6 * m_nApAntennas) * 8);
	int cbr = getPpduDuration(mcs, ru_size, (32 + 5 + cbr_ebr_fields) * 8);
	
	int maxRUs = getMaxRUsPerChannelWidth(bw, ru_size);
	
	//nSeq is the number of sequences in the sounding procedure. It depends on the number of streams (i.e. nApAntennas), 
	//the number of STAs and the number of of RUs. Examples:
	//1) 4 streams, 4 STAs and 1 RU (full bandwidth) : 1 sequence (the 4 STAs transmit their reports simultaneously)
	//2) 4 streams, 15 STAs and 1 RU : ceil(15/4) = ceil(3.75) = 4 sequences (4 then 4 then 4 then 3 reports)
	//3) 4 streams, 15 STAs and 2 RUs : ceil(15/(4*2)) = ceil(1.875) = 2 sequences (RU1: 4 then 4, RU2: 4 then 3)
	//4) 4 streams, 17 STAs and 2 RUs : ceil(17/(4*2)) = ceil(2.125) = 3 sequences (RU1: 4 then 4 then 1, RU2: 4 then 4 ==> max(3,2)=3seq)
	int nSeq = ceil(nBfees * 1.0/(m_nApAntennas * maxRUs));
	return ndpa + SIFS + ndp + (SIFS + brp + SIFS + cbr) * nSeq;
}



struct sta {
	//variables for RA STAs for channel contention
	int bt; //backoff time
	int cw; //contention window

	//variables for RA STAs for RA RU contention
	int obo; //OFDMA BackOff
	int ocw; //OFDMA contention window
	int usedRARU; //to determine if this station used a RA RU that experiences a collision
	
	//variables used for both channel and RA RU contentions
	long long int dequeueTime; //the dequeue time of an A-MPDU
	int nSuccAccesses; //this is not the number of MDPUs but the number of successfully transmitted A-MPDUs (over both the channel and the RUs)
	long long int sumDelays;
};

struct stats_struct {
	int nCollisions; //collisions on the different RA RUs
	int nNoCollisions; //successful transmissions on RA RUs
	int nRATx; //records the number of transmitted MPDUs on RA RUs. The total transmitted MPDUs = nRATx + nSATx
	int nSATx; //records the number of transmitted MPDUs on SA RUs. The total transmitted MPDUs = nRATx + nSATx
};


//"endTxTime" is the time in µs of the transmission end (including BACK). It is used to record the average transmission delay
void ul_ofdma(int nSARUs, int nRARUs, int nRAStas, sta *RAStas, stats_struct *ofdma_stats, int ofdma_ampdu_len, long long int endTxTime) {
	
	int *nRARUSenders = NULL; //records the number of senders on each RA RU to determine the collisions
	
	if (nRARUs > 0) {
		nRARUSenders = new int[nRARUs];
	}

	//initialise nRARUSenders
	for (int i = 0; i < nRARUs; i++) {
		nRARUSenders[i] = 0;
	}
	
	//contend and send using RA RUs
	for (int i = 0; i < nRAStas; i++) {
		if (RAStas[i].obo < 0) {
			//this case should not accur
			printf("Error : RAStas[i].obo < 0\nExit ...\n");
			exit(0);
		}
		
		//should check that nRARUs > 0, because RAStas[i].obo may be 0
		if (nRARUs > 0 && RAStas[i].obo <= nRARUs) {
			//send on a randomly selected RUs
			RAStas[i].obo = -1; //make it invalid so we can set it later
							    //do not set it here, because OBO depends on the transmission status (success or failure)
			int usedRARU = rand() % nRARUs;
			RAStas[i].usedRARU = usedRARU;
			nRARUSenders[usedRARU] += 1; //increase the number of senders on this RA RU to count the number of collisions
			
			//we are not sure about the success of the transmission, so we dont increment nRATx now
		}
		else {
			RAStas[i].obo -= nRARUs; //if nRARUs=0, RAStas will never send. This is fine !
		}
	}
	
	//handle collisions here
	for (int i = 0; i < nRARUs; i++) {
		if (nRARUSenders[i] > 1) {
			//there is a collision on this RA RU
			ofdma_stats->nCollisions += 1;
		}
		else if (nRARUSenders[i] == 1) {
			//one transmission on this RA RU (no collision) => just increase the number of A-MPDUs transmitted on RA RUs
			//even in case of UL MU-MIMO, an RA RU must be used by a single station because UL MU-MIMO requires scheduled 
			//transmission an is possible on SA RUs only
			//we suppose that any RA/SA STA will send the maximum number of MPDUs that a RU supports
			ofdma_stats->nRATx += ofdma_ampdu_len;
			ofdma_stats->nNoCollisions += 1;
		}
	}
	
	//handle OBO of RA STAs here
	for (int i = 0; i < nRAStas; i++) {
		if (RAStas[i].obo < 0) {
			//this station is sending: check if there is collision on the used RA RU
			int usedRARU = RAStas[i].usedRARU;
			
			if (nRARUSenders[usedRARU] < 1) {
			    //this case should not accur
			    printf("Error : nRARUSenders[usedRARU] < 1\nExit ...\n");
			    exit(0);
			}
			else if (nRARUSenders[usedRARU] == 1) {
				//no collisions: set a new OBO using an initialised window
				RAStas[i].ocw = OCW_MIN;
				RAStas[i].obo = rand() % (OCW_MIN + 1);
				
				
				//this transmission is successful: record its duration and save the dequeue time of the next A-MPDU
				long long int delay = endTxTime - RAStas[i].dequeueTime;
				RAStas[i].sumDelays += delay;
				RAStas[i].nSuccAccesses += 1;
				RAStas[i].dequeueTime = endTxTime;
			}
			else {
				//there is collision: increase the cw and select a new OBO
				RAStas[i].ocw = std::min((RAStas[i].ocw + 1) * 2 - 1, OCW_MAX);
				RAStas[i].obo = rand() % (RAStas[i].ocw + 1);
			}
		}
	}
	
	//send using SA RUs
	for (int i = 0; i < nSARUs; i++) {
		//each SA STA sends MPDUs subject to max PPDU duration
		//the maximum number of streams is transmitted at each SA RU
		ofdma_stats->nSATx += m_nApAntennas * ofdma_ampdu_len;
	}
	
	if (nRARUs > 0) {
		delete[] nRARUSenders;
	}
}


struct wlan_result simulate_wlan(const int bw, const int access_method, const int ru_size, int nRARUs, int nRAStas, int mcs, int max_ampdu_len) {
    //first, check params and print a summary
    int maxRUs = getMaxRUsPerChannelWidth(bw, ru_size);
	int nSARUs; //this is the number of SA RUs and also the number of SA STAs
	
	if (nRAStas < 0) nRAStas = 0;
	if (mcs < 0) mcs = 0;
	if (mcs > 11) mcs = 11;
	if (nRARUs < 0) nRARUs = 0;
	if (nRARUs > maxRUs) nRARUs = maxRUs;
	if (max_ampdu_len < 1) max_ampdu_len = 1;
	
	nSARUs = maxRUs - nRARUs;
	
    
#if 0
    printf("Channel Width                     : %d MHz\n", bw);
    printf("RU size                           : %d tones\n", ru_size);
    printf("Max number of RUs                 : %d\n", maxRUs);
    printf("Number of SA RUs (and SA STAs)    : %d\n", nSARUs);
    printf("Number of RA RUs                  : %d\n", nRARUs);
    printf("Number of STAs (Random Access)    : %d\n", nRAStas);
    printf("MCS index                         : %d\n", mcs);
#endif
    
    if (maxRUs <= 0) {
    	printf("maximum number of RUs (maxRUs=%d) should be > 0\n", maxRUs);
    	exit(0);
    }
    
    if (m_nApAntennas > 1 && ru_size < RU_SIZE_106_TONES) {
    	printf("both DL and UL MU-MIMO are possible on RU size >= 106 tones\n");
    	exit(0);
    }
    
    
    /*
    int abcd = getSoundingDuration(bw, mcs, ru_size, nRAStas);
    printf ("sounding duration = %d\n", abcd);
    exit(0);
    */
    
    

	/* initialize random seed: */
    srand (time(NULL)); //put it here because there is a variable called "time", to avoid the error : 'time' cannot be used as a function

	int nCollisions = 0;
	int nNoCollisions = 0;
	int nTx = 0; //this is the number of transmitted MPDUs using EDCA (UL OFDMA transmissions are excluded and are recorded using other variables)
	stats_struct ofdma_stats;
	ofdma_stats.nCollisions = 0;
	ofdma_stats.nNoCollisions = 0;
	ofdma_stats.nRATx = 0;
	ofdma_stats.nSATx = 0;
	
	sta APSta;
	sta *RAStas = NULL; //it is possible that nRAstas=0
	long long int time = 0; //time in µs
	
	int duration_tf = 44;
	int ofdma_ampdu_len = std::min(max_ampdu_len, getOfdmaAMpduLength(mcs, ru_size, MAX_PPDU_DURATION_US, ETHERNET_FRAME_LENGTH));
	int ofdma_ampdu_duration = getOfdmaAMpduDuration(mcs, ru_size, ofdma_ampdu_len, ETHERNET_FRAME_LENGTH);
	int duration_multi_sta_back = 44;
	

	bool waitAifs = true;
	int edca_ampdu_len = std::min(max_ampdu_len, getEdcaAMpduLength(mcs, bw, MAX_PPDU_DURATION_US, ETHERNET_FRAME_LENGTH));
	int edca_ampdu_duration = getEdcaAMpduDuration(mcs, bw, edca_ampdu_len, ETHERNET_FRAME_LENGTH);
	int duration_back = 44;
	
	/*
	printf ("max_ofdma_ampdu_len = %d, max_edca_ampdu_len = %d\n", 
			getOfdmaAMpduLength(mcs, ru_size, MAX_PPDU_DURATION_US, ETHERNET_FRAME_LENGTH), 
			getEdcaAMpduLength(mcs, bw, MAX_PPDU_DURATION_US, ETHERNET_FRAME_LENGTH));
    exit(0);
	*/
	
	int nextSoundingTime = 0;
	int sounding_duration = 0;
	if (access_method == DL_MU_MIMO_WITH_EDCA) {
		//this is required to be able to use a number of antennas different that 1
		sounding_duration = getSoundingDuration(bw, mcs, ru_size, m_nBfees);
	}
	
	if (nRAStas > 0) {
	    RAStas = new sta[nRAStas];
	}
	
	//set BT of AP as it will contend if UL OFDMA with EDCA is used (AP does not have data to send, so it does not contend in case of pure EDCA)
	APSta.bt = rand() % (CW_MIN + 1);
	APSta.cw = CW_MIN;
	APSta.dequeueTime = 0; //under high load condition, the first A-MPDU is dequeued at t=0
	APSta.nSuccAccesses = 0;
	APSta.sumDelays = 0;
	
	//set BT and OBO for all RAStas
	for (int i = 0; i < nRAStas; i++) {
		RAStas[i].bt = rand() % (CW_MIN + 1);
		RAStas[i].cw = CW_MIN;
		
		RAStas[i].obo = rand() % (OCW_MIN + 1);
		RAStas[i].ocw = OCW_MIN;
		RAStas[i].usedRARU = -1; //unvalid value
		
		RAStas[i].dequeueTime = 0; //under high load condition, the first A-MPDU is dequeued at t=0
		RAStas[i].nSuccAccesses = 0;
		RAStas[i].sumDelays = 0;
	}
	
	while (time < MAX_SIMULATION_TIME_US) {
		if (access_method == PURE_UL_OFDMA) {
			long long int txTime = duration_tf + SIFS + ofdma_ampdu_duration + SIFS + duration_multi_sta_back;
			ul_ofdma(nSARUs, nRARUs, nRAStas, RAStas, &ofdma_stats, ofdma_ampdu_len, time + txTime);
			time += txTime + SIFS;

			//no need for EDCA contention
			continue;
		}
		
		if (waitAifs) {
			time += AIFS;
			waitAifs = false;
		}
		
		//Check if there are senders. If yes, BT should not be decreased. So DO NOT decrement BT here
		int nSenders = 0;
		for (int i = 0; i < nRAStas; i++) {
			if (RAStas[i].bt < 0) {
				//this case should not accur
				printf("Error : RAStas[i].bt < 0\nExit ...\n");
				exit(0);
			}
			
			if (RAStas[i].bt == 0) {
				nSenders += 1;
				RAStas[i].bt = -1; //make it invalid so we can set it later
								   //do not set it here, because BT depends on the transmission status (success of failure)
			}
		}

		//check if the AP will send
		int ap_sending = false;
		if (access_method == UL_OFDMA_WITH_EDCA || access_method == DL_OFDMA_WITH_EDCA || access_method == DL_MU_MIMO_WITH_EDCA) {
			if (APSta.bt < 0) {
				//this case should not accur
				printf("Error : APSta.bt < 0\nExit ...\n");
				exit(0);
			}
			
			if (APSta.bt == 0) {
				ap_sending = true;
				nSenders += 1;
				APSta.bt = -1; //make it invalid so we can set it later
								   //do not set it here, because BT depends on the transmission status (success of failure)
			}
		}
		
		//if no senders, advance time with slottime. otherwise, advance time with the transmission duration
		if (nSenders == 0) {
			time += SLOT_TIME; //do not "continue", we should decrease BT
		}
		else if (nSenders == 1) {
			waitAifs = true;
			nNoCollisions += 1;
			
			if (ap_sending) {
				//no collisions, so simulate an AP transmission
				
				if (access_method == UL_OFDMA_WITH_EDCA) {
					long long int txTime = duration_tf + SIFS + ofdma_ampdu_duration + SIFS + duration_multi_sta_back;
					ul_ofdma(nSARUs, nRARUs, nRAStas, RAStas, &ofdma_stats, ofdma_ampdu_len, time + txTime);
					time += txTime;
					
					//do not increase the number of transmissions, this is already done by ul_ofdma()
				}
				else if (access_method == DL_OFDMA_WITH_EDCA) {
					nTx += maxRUs * ofdma_ampdu_len;
					time += ofdma_ampdu_duration + SIFS + duration_multi_sta_back;
					
					//this transmission is successful: record its duration and save the dequeue time of the next A-MPDU
					APSta.sumDelays += time - APSta.dequeueTime;
					APSta.nSuccAccesses += 1;
					APSta.dequeueTime = time;
				}
				else if (access_method == DL_MU_MIMO_WITH_EDCA) {
					//if this is time for channel sounding, just advance time and do not increase nTx
					if (time >= nextSoundingTime) {
						time += sounding_duration;
						nextSoundingTime = time + m_soundingPeriodMicros;
					}
					else {
						nTx += maxRUs * ofdma_ampdu_len * m_nApAntennas;
						time += ofdma_ampdu_duration + SIFS + duration_multi_sta_back;
						
						//this transmission is successful: record its duration and save the dequeue time of the next A-MPDU
						APSta.sumDelays += time - APSta.dequeueTime;
						APSta.nSuccAccesses += 1;
						APSta.dequeueTime = time;
					}
				}
				else {
					//this case should not accur
					printf("Wrong access_method\nExit ...\n");
					exit(0);
				}
			}
			else {
				//this is a transmission by a contending STA
				nTx += edca_ampdu_len;
				time += edca_ampdu_duration + SIFS + duration_back;
			}
		}
		else { //nSenders > 1
			waitAifs = true;
			nCollisions += 1;
			
			//if the AP is among the transmitter and is using UL OFDMA, it will stop any transmission 
			//after sending the TF so the wasted time is always equal to a station transmission duration
			//if the AP is using DL OFDMA or DL MU-MIMO, it must not come here (actually we consider no collisions in these cases)
			if (access_method == DL_OFDMA_WITH_EDCA || access_method == DL_MU_MIMO_WITH_EDCA) {
				//this case should not accur
				printf("We should not have collisions in case of DL OFDMA or DL MU-MIMO (we only measure DL throughput)\nExit ...\n");
				exit(0);
			}
			time += edca_ampdu_duration + SIFS + duration_back;
		}
		
		//if there is no transmission, decrease BT of STAs. Otherwise, do not decrease BT but initialize invalid BT
		for (int i = 0; i < nRAStas; i++) {
			if (nSenders == 0) {
				//there is no invalid BT, just decrement all BT
				if (RAStas[i].bt <= 0) {
					//this case should not accur
					printf("Error2 : RAStas[i].bt <= 0\nExit ...\n");
					exit(0);
				}
			
				RAStas[i].bt -= 1;
			}
			else if (nSenders == 1) {
				//there is 1 sender. initialise its CW, set its BT and "break" this "for" loop
				if (RAStas[i].bt < 0) {
					RAStas[i].cw = CW_MIN;
					RAStas[i].bt = rand() % (CW_MIN + 1);
					
					
					//this transmission is successful: record its duration and save the dequeue time of the next A-MPDU
					RAStas[i].sumDelays += time - RAStas[i].dequeueTime;
					RAStas[i].nSuccAccesses += 1;
					RAStas[i].dequeueTime = time;
					
					
					break; //"break" this "for" loop
				}
			}
			else {
				//there are more than 1 sender. find them, increase their CW, and set their BT
				if (RAStas[i].bt < 0) {
					RAStas[i].cw = std::min((RAStas[i].cw + 1) * 2 - 1, CW_MAX);
					RAStas[i].bt = rand() % (RAStas[i].cw + 1);
				}
			}
		}
		
		//if there is no transmission, decrease BT of AP. Otherwise, do not decrease BT but initialize BT if invalid
		//the AP contends for the channel in the following cases: UL_OFDMA_WITH_EDCA, DL_OFDMA_WITH_EDCA, DL_MU_MIMO_WITH_EDCA
		//the AP does not contend for the channel in PURE_EDCA (we consider that only the STAs contends) and in PURE_UL_OFDMA
		if (access_method == UL_OFDMA_WITH_EDCA || access_method == DL_OFDMA_WITH_EDCA || access_method == DL_MU_MIMO_WITH_EDCA) {
			if (nSenders == 0) {
				//there is no invalid BT, just decrement all BT
				if (APSta.bt <= 0) {
					//this case should not accur
					printf("Error2 : APSta.bt <= 0\nExit ...\n");
					exit(0);
				}
			
				APSta.bt -= 1;
			}
			else if (nSenders == 1) {
				//there is 1 sender. initialise its CW and set its BT
				if (APSta.bt < 0) {
					APSta.cw = CW_MIN;
					APSta.bt = rand() % (CW_MIN + 1);
				}
			}
			else {
				//there are more than 1 sender. if the AP is among the senders, increase its CW and set its BT
				if (APSta.bt < 0) {
					APSta.cw = std::min((APSta.cw + 1) * 2 - 1, CW_MAX);
					APSta.bt = rand() % (APSta.cw + 1);
				}
			}
		}
	}
	
	struct wlan_result result;
	
	double collision_rate = 0;
	if ((nCollisions + nNoCollisions) > 0) collision_rate = nCollisions * 100.0 / (nCollisions + nNoCollisions);

#if 0
	//print some results
	printf("EDCA :\n");
	printf("EDCA A-MPDU length : %d\n", edca_ampdu_len);
	printf("EDCA A-MPDU duration : %dus\n", edca_ampdu_duration);
	printf("Number of EDCA MPDUs : %d\n", nTx);
	printf("Number of successful EDCA transmissions (number of A-MPDUs) : %d\n", nNoCollisions);
	printf("Number of EDCA collisions : %d\n", nCollisions);
	printf("EDCA collision rate : %.2f%%\n", collision_rate);
#endif
	
	result.edca_ampdu_len = edca_ampdu_len;
	result.edca_ampdu_duration = edca_ampdu_duration;
	result.edca_n_tx_mpdu = nTx;
	result.edca_nNoCollisions = nNoCollisions;
	result.edca_nCollisions = nCollisions;
	result.edca_collision_rate = collision_rate;
	
	
	
	collision_rate = 0;
	if ((ofdma_stats.nCollisions + ofdma_stats.nNoCollisions) > 0) 
		collision_rate = ofdma_stats.nCollisions * 100.0 / (ofdma_stats.nCollisions + ofdma_stats.nNoCollisions);
	
	double throughput = ofdma_stats.nSATx + ofdma_stats.nRATx + nTx; //WLAN throughput, i.e. EDCA + UL OFDMA throughput
	throughput = throughput * 1500 * 8 / MAX_SIMULATION_TIME_US;
	
#if 0
	printf("\n\nOFDMA :\n");
	printf("OFDMA A-MPDU length : %d\n", ofdma_ampdu_len);
	printf("Number of SA transmitted MPDUs : %d\n", ofdma_stats.nSATx);
	printf("Number of sccessful RA transmitted MPDUs : %d\n", ofdma_stats.nRATx);
	
	printf("Number of collisions on RA RUs    : %d\n", ofdma_stats.nCollisions);
	printf("Number of no collisions on RA RUs : %d\n", ofdma_stats.nNoCollisions);
	
	printf("Collision rate : %.2f%%\n", collision_rate);

	printf("\n\nWLAN throughput : %.2f Mbps\n", throughput);
#endif
	
	result.ofdma_ampdu_len = ofdma_ampdu_len;
	result.ofdma_n_SA_tx_mpdus = ofdma_stats.nSATx;
	result.ofdma_n_RA_tx_mpdus = ofdma_stats.nRATx;
	result.ofdma_nNoCollisions = ofdma_stats.nCollisions;
	result.ofdma_nCollisions = ofdma_stats.nNoCollisions;
	result.ofdma_collision_rate = collision_rate;
	
	result.throughput = throughput;

	//record the average tx delays
	//average transmission delays of AP is valid only in cases of DL OFDMA and DL MU-MIMO
	result.avgApTxDelaysMicros = 0;
	if (APSta.nSuccAccesses > 0) {
		if (access_method != DL_OFDMA_WITH_EDCA && access_method != DL_MU_MIMO_WITH_EDCA) {
			//this case should never accur
			printf("APSta.nSuccAccesses > 0 while access_method=%d\nExit ...\n", access_method);
			exit(0);
		}
		
		result.avgApTxDelaysMicros = APSta.sumDelays / APSta.nSuccAccesses;
	}
	
	//average transmission delays of RA STAs (the average of one STA is enough)
	result.avgRAStasTxDelaysMicros = 0;
	if (nRAStas > 0 && RAStas[0].nSuccAccesses > 0) {
		result.avgRAStasTxDelaysMicros = RAStas[0].sumDelays / RAStas[0].nSuccAccesses;
	}

	
	if (nRAStas > 0) {
	    delete[] RAStas;
	}
	return result;
}

