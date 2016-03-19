all: abp.c abp_nak.c
	g++ -std=c++11 abp.c -o run_ABP
	g++ -std=c++11 abp_nak.c -o run_ABP_NAK
