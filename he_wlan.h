#define BANDWIDTH_20MHz                20
#define BANDWIDTH_40MHz                40
#define BANDWIDTH_80MHz                80
#define BANDWIDTH_160MHz               160


#define RU_SIZE_26_TONES               26
#define RU_SIZE_52_TONES               52
#define RU_SIZE_106_TONES              106
#define RU_SIZE_242_TONES              242
#define RU_SIZE_484_TONES              484
#define RU_SIZE_996_TONES              996
#define RU_SIZE_2x996_TONES            1992


#define MCS_0                          0
#define MCS_1                          1
#define MCS_2                          2
#define MCS_3                          3
#define MCS_4                          4
#define MCS_5                          5
#define MCS_6                          6
#define MCS_7                          7
#define MCS_8                          8
#define MCS_9                          9
#define MCS_10                         10
#define MCS_11                         11

//Access Methods
#define PURE_EDCA                      1
#define UL_OFDMA_WITH_EDCA             2
#define PURE_UL_OFDMA                  3
#define DL_OFDMA_WITH_EDCA             4
#define DL_MU_MIMO_WITH_EDCA           5

struct wlan_result {
	//EDCA results
	int edca_ampdu_len;
	int edca_ampdu_duration;
	int edca_n_tx_mpdu; //number of transmitted MPDUs without OFDMA
	int edca_nNoCollisions; //number of EDCA transmissions without collisions
	int edca_nCollisions;
	double edca_collision_rate;
	
	//OFDMA
	int ofdma_ampdu_len;
	int ofdma_n_SA_tx_mpdus; //number of transmitted MPDUs using OFDMA SA RUs
	int ofdma_n_RA_tx_mpdus;
	int ofdma_nNoCollisions; //number of OFDMA transmissions without collisions
	int ofdma_nCollisions;
	double ofdma_collision_rate;
	
	//network throughput
	double throughput;
	
	//average transmission delays (in µs) of AP (valid only in case of DL OFDMA and DL MU-MIMO)
	long long int avgApTxDelaysMicros;
	
	//average transmission delays (in µs) of STAs with random access (RA STAs)
	long long int avgRAStasTxDelaysMicros;
};


void setApAntennas(int nApAntennas);
void setSoundingPeriodMillis(int soundingPeriodMillis);
void setNBeamformees(int nBfees);
int getSoundingDuration(int bw, int mcs, int ru_size, int nStas);

/*
This function simulates a 802.11ax network and requires the bandwidth in MHz, the access method (PURE_EDCA, UL_OFDMA_WITH_EDCA, 
PURE_UL_OFDMA, DL_OFDMA_WITH_EDCA or DL_MU_MIMO_WITH_EDCA), the RU size, the number of RA RUs, the number of contending stations,
the mcs and the maximum A-MPDU length (i.e. number of oggregated MPDUs within an A-MPDU).
*/
struct wlan_result simulate_wlan(const int bw, const int access_method, const int ru_size, int nRARUs, int nRAStas, int mcs, int max_ampdu_len);

