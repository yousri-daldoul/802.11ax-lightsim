# 802.11ax lightsim

802.11ax lightsim is an open source simulator implemented in C++ to evaluate the performance of the major novelties of 802.11ax. Our tool does not simulate the transmission failures related to path loss, but is able to simulate the failures related to collisions. Therefore, it provides realistic results under the assumption that the selected MCS is suitable. Our simulator supports UL and DL OFDMA in addition to UL and DL MU-MIMO. It allows the measurement of throughput, delays and collision rates. It supports different channel widths, RU sizes and spatial streams. Besides, the simulator allows the selection of a variable number of contending stations. In the case of UL OFDMA and UL MU-MIMO, it is possible to configure the number of RA RUs and SA RUs. In the case of DL MU-MIMO, it is possible to evaluate the performance as a function of the channel sounding rate and the number of beamformees. Actually, the simulator does not implement any scheduling algorithm for UL multi-user transmissions. Therefore, we consider that the scheduled stations remain scheduled during the entire simulation time. In addition, the number of scheduled stations is not configurable and is equal to the number of SA RUs x number of AP antennas. This allows the SA RUs to be continuously used.

This simulator is mainly developed to study the performance of the recent features of 802.11ax. The results of this study are submitted for publication in the research paper entitled: “Performance Evaluation of OFDMA and MU-MIMO in 802.11ax Networks”. This paper is authored by Dr. Yousri Daldoul, Dr. Djamal-Eddine MEDDOUR and Dr. Adlen KSENTINI. It is actually under review in Computer Networks Journal. Therefore, the entire source code of the simulator will be available on this repository upon the publication of the paper with a reference of the paper.

The main function that allows the simulation of 802.11ax networks is:
**struct wlan_result simulate_wlan(const int bw, const int access_method, const int ru_size, int nRARUs, int nRAStas, int mcs, int max_ampdu_len);**
This function requires the bandwidth in MHz, the access method (PURE_EDCA, UL_OFDMA_WITH_EDCA, PURE_UL_OFDMA, DL_OFDMA_WITH_EDCA or DL_MU_MIMO_WITH_EDCA), the RU size, the number of RA RUs, the number of contending stations, the MCS and the maximum A-MPDU length (i.e. number of aggregated MPDUs within an A-MPDU).

* **simulate_wlan()** is used by the following functions to obtain the results of the aforementioned paper:  
* **void pureOfdmaCollisionRate():** This function measures the collision rate of pure UL OFDMA as a function of the number of contending stations and the number of RA RUs. It is used to build Figure 6 of the paper.  
**void pureOfdmaThroughput():** This function measures the throughput of pure UL OFDMA as a function of the number of contending stations and the number of RA RUs. It is used to build Figure 7 of the paper.  
**void pureOfdmaDelays():** This function measures the average transmission delays of pure UL OFDMA as a function of the number of contending stations and the number of RA RUs. It is used to build Figure 8 of the paper.  
**void throughputComparison1():** This function sets the number of stations to 16 (number of scheduled + unscheduled stations = 16) and compares the throughput of legacy full bandwidth transmissions (i.e. pure EDCA), "UL OFDMA with EDCA" and pure UL OFDMA as a function of the number of of RA RUs. It is used to build Figure 9 of the paper.  
**void ulThroughputComparison2():** This function compares the throughput of legacy full bandwidth transmissions (i.e. pure EDCA), "UL OFDMA with EDCA" and pure UL OFDMA as a function of the number of contending stations. It is used to build Figure 10 of the paper.  
**void ulDelayComparison():** This function compares the average transmission delays of legacy full bandwidth transmissions (i.e. pure EDCA), "UL OFDMA with EDCA" and pure UL OFDMA as a function of the number of contending stations. It is used to build Figure 11 of the paper.  
**void ulThroughputComparison3():** This function compares the throughput of legacy full bandwidth transmissions (i.e. pure EDCA), "UL OFDMA with EDCA" and pure UL OFDMA as a function of the maximum A-MPDU length (i.e. number of aggregated MPDUs within an A-MPDU). It is used to build Figure 12 of the paper.  
**void dlOfdmaThroughput():** This function compares the throughput of legacy full bandwidth transmissions (i.e. pure EDCA) and DL OFDMA as a function of the maximum A-MPDU length (i.e. number of aggregated MPDUs within an A-MPDU). It is used to build Figure 13 of the paper.  
**void ulMuMimoThroughput1():** This function compares the throughput of legacy full bandwidth transmissions (i.e. pure EDCA) and UL MU-MIMO as a function of the maximum A-MPDU length and the number of spatial streams. It is used to build Figure 14 of the paper.  
**void ulMuMimoThroughput2():** This function compares the throughput of legacy full bandwidth transmissions (i.e. pure EDCA) and UL MU-MIMO as a function of the maximum A-MPDU length and the RU size. It is used to build Figure 15 of the paper.  
**void ulMuMimoThroughput3():** This function compares the throughput of legacy full bandwidth transmissions (i.e. pure EDCA) and UL MU-MIMO as a function of the number of contending stations and the RU size. It is used to build Figure 16 of the paper.  
**void dlMuMimoThroughput1():** This function compares the throughput of legacy full bandwidth transmissions (i.e. pure EDCA) and DL MU-MIMO as a function of the sounding period and the RU size. It is used to build Figure 17 of the paper.  
**void dlMuMimoDelays1():** This function compares the average transmission delays of legacy full bandwidth transmissions (i.e. pure EDCA) and DL MU-MIMO as a function of the sounding period and the RU size. It is used to build Figure 18 of the paper.  
**void dlMuMimoThroughput2():** This function compares the throughput of legacy full bandwidth transmissions (i.e. pure EDCA) and DL MU-MIMO as a function of the number of beamformees and the RU size. It is used to build Figure 19 of the paper.  
**void dlMuMimoThroughput3():** This function compares the throughput of legacy full bandwidth transmissions (i.e. pure EDCA) and DL MU-MIMO as a function of the maximum A-MPDU length and the RU size. It is used to build Figure 20 of the paper.  











