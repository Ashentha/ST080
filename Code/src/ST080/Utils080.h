/*
 * The MIT License (MIT)

Copyright (c) 2015 Tinotenda Chemvura, John Odetokun, Othniel Konan, Herman Kouassi

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

 */

/*
 * Utils080.h
 *
 *  Created on: Sep 26, 2016
 *      Author: Tinotenda Chemvura
 */

#ifndef UTILS080_H_
#define UTILS080_H_

// other stuff
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

// Library 38 (interrupts library)
#include "TM38/defines.h"
#include "TM38/tm_stm32f4_disco.h"
#include "TM38/tm_stm32f4_exti.h"

// FreeRTOS stuff
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

// STM32f4 stuff
#include "stm32f4xx.h"
#include "stm32f4_discovery.h"

// LCD library
#include "LCD/tm_stm32f4_hd44780.h"

// EEprom library
#include "Eeprom.h"

// Tempo library
#include "Tempo.h"

// ==========================================================================================
// ============================ FreeRTOS stuff ==============================================
// ==========================================================================================

uint64_t tickTime = 0;        // Counts OS ticks (default = 1000Hz).
uint64_t u64IdleTicks = 0;   // Value of u64IdleTicksCnt is copied once per sec.
uint64_t u64IdleTicksCnt = 0; // Counts when the OS has no task to execute.
uint16_t u16PWM1 = 0;

// This FreeRTOS callback function gets called once per tick (default = 1000Hz).
// ----------------------------------------------------------------------------
void vApplicationTickHook(void) {
	++tickTime;
}

// This FreeRTOS call-back function gets when no other task is ready to execute.
// On a completely unloaded system this is getting called at over 2.5MHz!
// ----------------------------------------------------------------------------
void vApplicationIdleHook(void) {
	++u64IdleTicksCnt;
}

// A required FreeRTOS function.
// ----------------------------------------------------------------------------
void vApplicationMallocFailedHook(void) {
	configASSERT(0);  				// Latch on any failure / error.
}

// ==========================================================================================
// ============================ Global Variables ============================================
// ==========================================================================================

// define task priorities
#define MODES_TASK_PRIORITY 1
#define UI_TASK_PRIORITY 2
#define GPIO_TASK_PRIORITY 3

// define task stack sizes
#define MODES_STACK_SIZE 128
#define UI_STACK_SIZE 128
#define GPIO_STACK_SIZE 128

// LCD dimensions
#define LCD_COLUMNS 16
#define LCD_ROWS 2

// Modes used by the ST080
#define COMPOSER 1
#define PLAYBACK 2
#define FREESTYLE 3
#define SAVE 4
#define ERROR_MODE 5
#define ENTER 6

// Instrument Macros
#define INSTR_1 0
#define INSTR_2 1
#define INSTR_3 2
#define INSTR_4 3

#define SAMPLE_SIZE 1103
#define DEFAULT_COMPOSER_BUFFERSIZE 20000
#define DEFAULT_TEMPO 0 // define proper tempo
#define CHANNEL_RACK_SIZE 1024

bool PAD_STATE[4];  				//Global variable use to check the state of the Freestyle pad
bool STATE_CHANGED;					//Global variable to check if there has been a change in Pad status
uint8_t MODE = COMPOSER; 			// Global variable to e used to identify which mode the ST080 is currently in
bool status = true;					// Variable used by Composer to check if there has been an update on the channel rack configs.
uint64_t current = 0, previous = 0;	// Variables to be used by IRQ Handler for debouncing
uint8_t current_sample = INSTR_1;	// variable used by the composer mode to check which instrument sample is on the channel rack
uint8_t played_inst = 0;			// variable used by the Freestyle mode to determine the instrument(s) played
uint16_t freestyle_samples [11][SAMPLE_SIZE]; // This will hold the samples of the different possible combinations
bool channelRack[16][4][16]; 		// 16 channel racks with 4 instruments each with 16 beat channel
uint8_t currentBeat = 0;			// Variable to indicate the current beat/instrumental being edited on the beat rack.
bool resetLEDs = true;				// flag used to fresh the LEDs when switching modes. This flag will be checked by the UI_Task to check whether it should reset the LEDs or not
bool UPDATE_LCD = true; 				// flag used to update LCD
bool UPDATE_BEAT = true;			//flag used to update beat on LCD in playback mode
bool UPDATE_TEMPO = true;
uint16_t ComposerBuffer[DEFAULT_COMPOSER_BUFFERSIZE];		// Buffer used by the composer mode to push to the audio output interface
uint16_t tempo = DEFAULT_TEMPO;
uint16_t drumKit1 [4][SAMPLE_SIZE] = {
/*clap*/			{2035,1625,1537,1962,1848,1871,2569,2581,1572,1207,2445,3206,2725,2460,1543,330,1164,3305,3319,1241,596,1320,1725,2148,1994,2687,3972,3459,2655,2203,1997,2089,927,0,1856,3745,3354,2671,2197,2018,2034,594,145,1136,1723,2122,2105,2149,2038,2198,3687,3915,1550,348,1059,1623,2087,2062,2406,3890,3719,2776,2330,886,0,1810,3741,3288,2715,1985,354,316,1338,1736,3173,4072,3177,2573,1740,160,620,3026,3668,2903,2422,2006,2128,1460,151,721,1482,2023,3582,3879,2879,2444,1265,22,1261,3500,3242,1035,593,1335,1717,3056,4072,2503,485,1169,3332,3303,1163,561,2512,3716,2089,504,1028,1506,2255,3773,3622,2729,2297,927,113,1815,3668,2784,713,981,3076,3463,1417,549,1217,1686,2083,2026,2448,3801,3354,1115,588,1278,1776,2083,2103,2116,2062,2071,2053,2059,2073,2029,3106,3920,2243,524,1366,3395,3099,1078,696,1377,1805,2095,2084,2117,2045,2081,2021,2177,3506,3618,2737,2382,1192,255,1041,1583,2300,3687,3512,2693,2239,1989,2034,2000,2093,2025,2122,1867,562,585,1415,1827,2127,2044,2138,1949,2400,3635,3370,2603,2208,1978,2048,1973,2152,1547,347,1121,3165,3261,1362,639,2346,3509,2220,597,1483,3254,3191,2602,2205,2012,2026,1989,2122,1475,319,1311,3197,3264,2629,2234,1984,2069,1245,303,1613,3320,3161,2557,2187,1970,2061,996,446,1201,1720,2049,2082,2080,2053,2028,2053,2010,2094,1945,2650,3662,3097,2494,2114,1997,2030,952,501,1322,1684,2560,3668,3158,2538,2044,761,545,1395,1720,2770,3667,3027,2483,1900,615,702,2632,3357,2789,2346,2046,2011,1998,2039,2041,2045,2046,1018,474,2086,3356,2914,2484,1855,571,840,2712,3338,1767,695,1915,3354,2555,859,1149,2953,3202,1475,696,2195,3434,2287,776,1089,1549,2071,3379,3441,2680,2257,1987,2027,1978,2105,1802,620,809,1503,1877,2996,3671,2158,755,1094,1602,1966,2072,2086,2062,2044,2040,2035,2042,2043,2044,2048,2042,2057,2032,2078,1983,2775,3625,2448,821,1275,3002,3148,2603,2248,1998,2091,1383,491,1510,3211,2900,1206,891,1503,1756,2906,3504,2874,2348,2054,1990,2010,2024,2052,2038,2055,2031,2051,2026,2049,2026,2051,1973,888,727,1438,1803,2104,1988,2447,3426,3064,2470,2105,942,682,1387,1797,2094,2024,2356,3411,2999,1299,871,2437,3198,2726,2335,2057,2016,2003,2035,2042,2044,2048,2030,2053,2010,2093,1693,645,1197,2852,3121,2563,2266,1270,574,1765,3193,2638,1080,1175,2730,3112,2561,2314,1348,653,1236,1705,2029,2066,2127,3113,3239,2600,2225,2005,2009,2008,2041,2046,2038,2058,2010,2097,1288,713,1343,1697,2295,3332,2963,1333,973,1478,1820,2057,2049,2094,1995,2242,3242,3008,1450,932,2292,3128,2673,2389,1740,733,1228,2803,3039,2556,2218,2017,2013,1999,2047,2021,2069,1938,978,929,2399,3164,2028,971,1356,1648,2197,3194,3071,2469,2178,1955,2082,1456,796,1283,1727,2012,2067,2091,2942,3222,1878,959,1388,1678,2272,3219,2999,2445,2146,1983,2035,1977,2121,1641,851,1215,1696,1972,2087,2037,2791,3249,2127,967,1628,2908,2845,2440,2122,1157,868,2166,3068,2662,2384,1814,898,1111,1644,1936,2861,3216,2044,968,1742,2915,2791,2397,2131,2005,2032,1318,852,1973,3016,2721,2391,1952,998,1055,1600,1897,2094,2009,2475,3216,2519,1219,1215,1633,1926,2811,3186,2025,1038,1733,2937,2567,1324,1217,1642,1890,2085,2000,2556,3157,2688,2344,1925,1038,1100,1641,1890,2683,3193,2235,1093,1585,2786,2805,2410,2162,1275,986,1528,1802,2389,3174,2604,1328,1227,1640,1900,2071,2043,2095,1980,2335,3070,2794,2367,2083,1231,1018,2158,3002,2211,1175,1616,2793,2678,1486,1222,1626,1867,2074,2007,2394,3097,2562,1330,1319,2500,2879,1814,1174,1559,1761,2314,3046,2788,2350,2109,1988,2048,1448,1005,1890,2920,2442,1340,1334,1694,1931,2060,2049,2069,2019,2063,1989,2218,2951,2822,2364,2157,1375,1075,1533,1856,2052,2067,2069,2039,2064,2763,2955,1901,1158,1904,2824,2611,2349,2001,1211,1191,1672,1903,2101,2002,2395,2978,2677,2301,2084,2006,2029,2011,2089,1625,1059,1745,2780,2566,1476,1362,2406,2796,2441,2223,2012,2068,1798,1155,1349,1749,1961,2083,2017,2520,2982,2247,1268,1606,2632,2698,2364,2157,1464,1107,2010,2801,2572,2294,2074,2029,1989,1350,1221,1661,1896,2070,2045,2077,2011,2127,2787,2792,2362,2178,1569,1110,1844,2748,2602,2327,2047,1319,1253,1678,1911,2080,2023,2297,2908,2518,1501,1376,1725,1902,2561,2897,2489,2252,1851,1190,1483,2515,2713,2382,2176,2010,2064,1737,1181,1637,2624,2602,1644,1374,1705,1862,2418,2906,2307,1379,1597,2530,2652,2335,2173,1547,1226,1619,1839,2230,2853,2572,1580,1412,2288,2730,1975,1370,1627,1812,2172,2796,2673,2307,2122,1495,1267,1648,1889,2063,2037,2173,2750,2646,1695,1394,1692,1896,2061,2027,2244,2801,2531,1597,1452,2281,2712,1976,1419,1632,1860,2022,2061,2061,2057,2036,2529,2795,2057,1379,1821,2627,2432,1583,1521,2356,2649,2351,2215,1739,1290,1580,1831,2100,2691,2683,2314,2160,1631,1307,1631,1880,2044,2064,2064,2053,2032,2510,2733,2382,2190,2021,2057,1921,1406,1474,1797,1973,2069,2054,2063,2032,2052,2021,2113,2644,2621,1802,1437,2085,2652,2175,1485,1743,2522,2482,1714,1495,2234,2607,2374,2193,2043,2054,1929,1429,1524,2346,2600,2339,2185,1778,1326,1792,2524,2516,2263,2113,2018,2042,2006,2085,1797,1397,1616,1863,2015,2061,2055,2050,2035,2044,2033,2048,2037,2053,2037,2075,2537,2631,2298,2181,1761,1407,1651,1877,2096,2615,2574,1828,1509,1769,1886,2276,2673,2461,2222,2067,2027,2026,1645,1424,2066,2557,2405,2202,2070,2027,2031,2034,2049,2040,2057,2032,2073,1897,1453,1679,2392,2500,1839,1542,2093,2563,2134,1576,1685,1862,2051,2537,2562,1892,1524,2046,2523,2357,2216,1975,1515,1573,1848,1972,2417,2606,2346,2156,2043,2028,2035,2041,2056,2036,2075,1730,1498,1746,1923,2044,2055,2059,2046,2040,2048,2023,2302,2621,2177,1623,1682,1878,2004,2066,2049,2379,2591,2086,1579,1855,2448},
/*kick*/			{2034,2057,2028,2016,1962,2021,2087,2048,2416,2401,2382,1916,1717,2092,2254,2139,2201,2166,1863,1631,1658,1872,1920,1999,1980,1863,2025,1659,1916,1801,1870,2121,1827,2577,1567,2722,2319,934,2677,3016,2145,1300,1485,2687,3795,3419,2851,1307,1087,2961,3151,1274,552,1919,2021,1991,2802,2820,2812,2986,3118,2857,2461,2731,2726,2533,2330,2298,1869,1732,1621,1394,1809,1479,1198,996,1300,1171,848,1041,746,735,819,1114,1026,1196,1068,1176,1425,857,1409,1479,1420,1841,2019,2049,2164,2031,1697,2292,2849,2685,2478,2595,2884,2846,2474,2741,2566,2339,2441,2476,2952,2907,3001,3108,3017,3217,3416,3283,3370,3514,3297,3690,3522,3538,3540,3400,3381,3439,3343,3122,3153,2818,3076,2564,2757,2510,2180,2513,2049,1984,1879,1620,1256,1501,1207,1075,944,1016,952,350,774,201,277,372,321,509,99,443,240,368,536,484,471,486,462,561,550,634,803,646,1089,900,750,1137,1069,1084,1325,1402,1457,1411,1740,1619,1834,2114,2172,2176,2293,2542,2809,2799,2773,3428,3013,3509,3524,3651,3758,3788,3903,3808,4042,3831,4045,3862,3975,3934,3941,3920,3857,3906,3827,3778,3770,3619,3676,3558,3505,3462,3395,3323,3199,3157,3033,3002,2705,2771,2573,2470,2385,2334,2064,1998,1936,1736,1660,1505,1174,1085,1016,657,824,482,580,312,290,326,130,386,122,205,152,119,151,84,114,115,144,118,137,156,167,198,232,258,301,373,388,430,501,556,584,751,727,844,897,974,1082,1201,1278,1318,1623,1518,1717,1744,1964,1893,2255,2294,2207,2646,2485,2832,2894,3062,3091,3157,3358,3240,3264,3521,3231,3454,3530,3428,3561,3423,3465,3493,3573,3376,3567,3456,3481,3483,3518,3416,3457,3427,3409,3503,3311,3435,3259,3413,3238,3321,3209,3175,3237,3138,3144,3001,3051,3093,2701,2935,2965,2512,2960,2513,2799,2594,2437,2374,2423,2382,2263,2274,2223,2183,2036,2338,1577,2315,1674,1834,1949,1625,1792,1293,1905,1426,1428,1629,1390,1168,1457,986,1203,1190,711,1381,1022,743,970,890,749,728,841,663,624,850,591,748,642,620,722,679,628,696,662,638,724,621,800,697,762,651,828,757,764,879,854,902,929,970,1014,1091,1053,1218,1135,1345,1256,1428,1511,1504,1510,1681,1658,1767,1940,1904,1986,1994,2265,2076,2411,2391,2376,2455,2687,2482,2747,2922,2734,2918,2967,2946,3055,3201,2868,3512,3180,3176,3551,3252,3314,3618,3419,3399,3562,3590,3399,3488,3690,3331,3625,3517,3497,3664,3481,3449,3565,3469,3509,3600,3483,3339,3572,3376,3439,3509,3329,3425,3323,3404,3207,3261,3249,3188,3152,3226,3020,3144,3002,2896,3046,2530,3025,2675,2615,2853,2497,2684,2514,2476,2415,2365,2247,2188,2078,2166,1939,2050,1900,1633,2110,1426,1904,1507,1428,1580,1130,1685,1230,1429,1025,1320,1231,939,1119,1099,870,849,1147,623,1084,764,760,834,897,714,743,941,576,879,737,723,829,849,789,823,841,759,807,904,737,1053,921,786,1091,842,960,1071,956,991,1101,1011,1208,1095,1159,1317,1101,1344,1331,1309,1483,1338,1578,1645,1453,1629,1698,1761,1629,1897,1870,1924,1881,2013,2088,1955,2195,1987,2347,2200,2389,2149,2613,2264,2278,2752,2305,2759,2413,2953,2421,2840,2495,2826,2915,2647,2930,2782,3112,2466,3204,2593,3177,2740,3092,3092,2814,3166,2653,3217,2885,3083,2909,3239,2831,3039,3094,3082,3039,3021,2975,3133,3005,2783,3302,2709,3063,2949,3062,2923,2802,3017,2959,2821,2920,2879,2765,2968,2724,2813,2748,2751,2758,2775,2654,2638,2674,2659,2491,2669,2522,2499,2581,2325,2495,2501,2421,2172,2594,2183,2250,2319,2200,2222,2169,2070,2079,2211,1854,2152,1621,2158,1840,1782,1902,1559,1875,1657,1426,1732,1727,1184,1766,1304,1474,1219,1373,1410,1066,1375,1011,1261,1052,1213,1233,960,1184,1011,1144,1200,794,1185,1099,1015,1093,1143,973,1231,1068,1072,1232,1064,1188,1096,1425,957,1422,1214,1243,1482,1236,1474,1260,1366,1440,1403,1432,1241,1647,1403,1357,1695,1278,1502,1415,1658,1455,1464,1530,1556,1433,1586,1562,1560,1576,1493,1807,1453,1769,1530,1669,1647,1584,1742,1570,1616,1687,1671,1660,1845,1570,1854,1719,1627,1945,1660,1832,1870,1883,1961,1818,1900,1984,1940,2054,2032,2043,2043,2155,2100,2072,2248,2159,2378,2162,2304,2271,2343,2373,2409,2337,2496,2557,2505,2548,2529,2781,2394,2943,2483,2855,2754,2718,2987,2621,2912,2806,3037,2856,2973,2964,2917,2956,3026,2914,3119,2930,3049,3016,3040,3034,2937,3070,2932,2959,3091,2997,2865,3112,2707,3094,2754,2850,2943,2949,2767,2816,2769,2683,2875,2552,2883,2431,2838,2462,2599,2565,2337,2608,2274,2599,2319,2350,2304,2273,2220,2174,2213,2093,2283,1913,2178,2088,1917,1866,1997,1980,1688,2062,1742,1756,1860,1782,1643,1829,1606,1766,1707,1633,1590,1604,1722,1442,1732,1539,1518,1640,1506,1516,1620,1394,1608,1416,1614,1340,1627,1548,1383,1601,1321,1643,1314,1717,1298,1641,1319,1579,1577,1292,1743,1257,1849,1230,1691,1424,1525,1613,1367,1781,1374,1633,1437,1741,1324,1802,1505,1529,1641,1538,1709,1481,1679,1573,1773,1478,1871,1355,1862,1578,1773,1703,1650,1791,1617,1900,1506,1979,1634,2020,1599,2028,1710,1795,1904,1788,1917,1822,1995,1723,2115,1717,2133,1763,2074,1838,2075,1987,2003,2162,1821,2254,1879,2247,1967,2239,2059,2190,2071,2248,2203,2108,2265,2121,2317,2105,2362,2142,2258,2327,2187,2307,2326,2187,2384,2309,2339,2316,2415,2325,2343,2387,2359,2437,2308,2419,2285,2452,2290,2427,2408,2300,2439,2378,2402,2322,2423,2351,2383,2298,2471,2419,2328,2398,2285,2430,2301,2429,2321,2424,2279,2413,2360,2213,2448,2271,2409,2257,2406,2259,2292,2266,2285,2324,2229,2333,2159,2339,2171,2322,2173,2222,2265,2131,2392,2000,2316,2112,2168,2213,2035,2367,1974,2280,2107,2159,2119,2105,2148,2053,2191,2035,2216,2042,2207,1964,2278,2035,2105,2183,1980,2183,2067,2118,2034,2196,1948,2218,2014,2154,1998,2115,2089,1986,2224,1906},
/*cow bell*/		{2064,2244,2345,2248,2230,1991,1939,1192,585,1405,2398,2967,3205,2928,2659,1326,286,844,1606,2693,3171,3296,3072,2938,2426,1697,1049,169,0,0,1084,3000,4030,4094,4094,4023,3314,2224,1231,497,0,26,0,197,1542,3000,3991,3770,3666,3829,3541,2867,1863,1003,314,60,0,665,2136,3427,4038,4094,3770,3226,2309,1345,444,0,4,81,1359,2942,3977,4094,4094,3879,3030,2132,1245,607,74,89,0,621,1841,2995,3647,3463,3565,3466,3166,2475,1712,989,617,232,204,1182,2268,3214,3628,3683,3283,2882,2133,1487,648,40,340,617,1687,2753,3532,3887,3770,3296,2677,2075,1482,989,621,637,499,1030,1917,2765,3090,3079,3263,3128,2919,2354,1793,1220,957,665,656,1420,2240,2906,3222,3246,2963,2661,2121,1640,903,606,883,1046,1816,2560,3125,3360,3279,2919,2487,2092,1668,1274,1004,993,898,1264,1946,2550,2721,2868,3029,2920,2754,2296,1855,1389,1185,947,956,1568,2201,2692,2951,2958,2765,2518,2135,1710,1099,1002,1204,1316,1881,2431,2853,3020,2963,2684,2381,2112,1780,1459,1249,1224,1156,1411,1970,2382,2520,2745,2859,2780,2632,2256,1887,1502,1334,1141,1162,1669,2165,2554,2767,2769,2630,2422,2142,1725,1261,1270,1404,1491,1908,2342,2663,2797,2751,2534,2321,2128,1844,1581,1402,1376,1322,1509,1981,2249,2415,2666,2744,2686,2542,2228,1902,1579,1428,1276,1304,1735,2135,2464,2640,2650,2539,2367,2143,1729,1404,1445,1540,1604,1918,2278,2531,2648,2610,2443,2288,2140,1886,1667,1502,1483,1427,1581,1973,2152,2370,2609,2669,2619,2479,2204,1907,1629,1491,1371,1408,1779,2111,2402,2550,2573,2470,2336,2123,1727,1527,1562,1635,1675,1925,2227,2435,2543,2508,2383,2272,2146,1913,1725,1569,1560,1496,1641,1950,2094,2362,2568,2622,2567,2433,2180,1907,1659,1533,1443,1488,1812,2100,2362,2489,2520,2419,2320,2085,1737,1630,1645,1712,1722,1930,2187,2366,2463,2437,2346,2266,2147,1934,1764,1618,1615,1548,1686,1914,2071,2366,2539,2593,2523,2397,2155,1905,1677,1564,1501,1549,1828,2091,2328,2445,2479,2385,2303,2040,1769,1711,1706,1767,1752,1928,2148,2312,2401,2387,2328,2263,2144,1954,1792,1660,1652,1592,1708,1874,2076,2369,2520,2569,2489,2368,2133,1902,1686,1590,1552,1595,1838,2087,2298,2418,2445,2369,2280,2005,1819,1771,1756,1805,1771,1922,2114,2267,2349,2353,2319,2260,2143,1969,1810,1694,1677,1633,1708,1846,2094,2365,2507,2544,2460,2341,2113,1898,1693,1615,1593,1626,1846,2081,2275,2397,2415,2359,2242,1987,1868,1811,1796,1826,1783,1914,2088,2232,2312,2332,2314,2253,2140,1977,1824,1719,1696,1665,1696,1844,2121,2365,2501,2521,2436,2313,2095,1890,1699,1640,1629,1652,1858,2078,2261,2379,2393,2349,2198,1989,1910,1844,1829,1840,1794,1902,2067,2201,2283,2319,2310,2243,2139,1981,1839,1738,1715,1688,1678,1858,2140,2363,2491,2497,2412,2284,2080,1882,1709,1664,1660,1669,1864,2069,2248,2359,2379,2331,2157,2010,1945,1876,1853,1852,1800,1887,2045,2171,2261,2312,2308,2237,2144,1988,1858,1752,1739,1695,1663,1880,2150,2362,2480,2477,2391,2263,2071,1878,1722,1690,1685,1683,1870,2061,2240,2340,2372,2304,2132,2040,1973,1904,1871,1859,1800,1874,2026,2146,2247,2308,2303,2232,2144,1990,1872,1763,1759,1687,1662,1912,2160,2365,2465,2459,2368,2241,2056,1872,1736,1714,1703,1699,1879,2060,2236,2327,2368,2268,2123,2070,1994,1929,1881,1864,1797,1867,2012,2131,2241,2308,2295,2227,2140,1994,1882,1775,1775,1673,1680,1943,2171,2368,2450,2440,2342,2221,2040,1869,1752,1735,1717,1716,1882,2060,2227,2318,2353,2226,2129,2092,2011,1946,1886,1866,1790,1858,1993,2117,2237,2305,2285,2223,2135,2000,1886,1790,1778,1659,1708,1965,2181,2364,2434,2418,2315,2200,2025,1871,1770,1754,1731,1730,1883,2060,2214,2312,2327,2196,2151,2114,2032,1961,1893,1866,1785,1847,1977,2108,2237,2300,2278,2220,2132,2007,1891,1808,1767,1656,1745,1981,2190,2355,2416,2394,2292,2181,2015,1878,1790,1767,1744,1742,1885,2061,2205,2309,2293,2182,2174,2127,2050,1967,1896,1860,1780,1837,1967,2104,2237,2292,2272,2214,2130,2012,1897,1821,1751,1671,1778,1996,2198,2345,2401,2370,2272,2163,2008,1888,1809,1778,1758,1752,1892,2060,2201,2301,2257,2186,2191,2141,2063,1972,1898,1854,1778,1830,1961,2106,2237,2285,2268,2208,2132,2014,1908,1826,1737,1699,1804,2009,2199,2332,2382,2346,2252,2146,2006,1901,1825,1788,1773,1760,1897,2054,2197,2280,2229,2201,2204,2155,2073,1977,1898,1850,1776,1821,1957,2107,2234,2276,2264,2202,2135,2016,1922,1821,1733,1733,1824,2021,2195,2318,2361,2325,2235,2133,2009,1916,1837,1798,1784,1767,1902,2049,2196,2254,2214,2218,2213,2168,2077,1979,1895,1847,1773,1816,1957,2109,2228,2270,2257,2198,2136,2017,1933,1808,1739,1761,1841,2031,2189,2305,2340,2304,2216,2122,2012,1927,1845,1807,1791,1777,1906,2049,2191,2223,2210,2230,2219,2173,2076,1977,1888,1841,1770,1816,1962,2113,2221,2264,2249,2195,2132,2021,1934,1791,1758,1790,1861,2042,2184,2293,2319,2285,2199,2114,2018,1938,1853,1818,1800,1789,1909,2051,2180,2196,2217,2239,2225,2176,2075,1976,1883,1839,1769,1818,1967,2113,2214,2258,2240,2192,2128,2028,1928,1783,1785,1816,1880,2045,2175,2274,2297,2265,2182,2109,2027,1947,1861,1827,1807,1799,1908,2054,2156,2177,2231,2246,2233,2178,2074,1972,1878,1834,1767,1822,1974,2111,2208,2253,2235,2192,2127,2036,1914,1787,1815,1836,1897,2044,2167,2256,2277,2246,2169,2110,2037,1953,1871,1835,1817,1809,1912,2056,2131,2174,2243,2252,2239,2175,2072,1965,1873,1830,1770,1832,1983,2109,2205,2244,2230,2188,2126,2036,1896,1804,1840,1856,1913,2045,2161,2239,2260,2229,2160,2112,2043,1957,1879,1841,1828,1818,1921,2051,2110,2181,2250,2256,2239,2171,2067,1959,1871,1828,1777,1844,1988,2106,2201,2236,2227,2182,2129,2030,1887,1828,1862,1876,1922,2044,2150,2222,2242,2213,2154,2116,2048,1962,1888,1847,1840,1825,1928,2037,2095,2190,2254,2259,},
/*open hat*/		{2726,1665,2040,2877,1314,1152,1956,3836,2808,1674,231,3199,1976,893,2749,2396,2822,714,242,4091,3219,582,2292,2331,2335,1470,2818,729,2456,3455,1249,1148,2659,2800,833,2717,2708,1781,1047,2750,2352,1685,2325,2151,1952,2483,1051,1740,2475,2114,1333,1762,3214,1224,1446,2744,2863,1608,2150,1892,1567,2260,1692,1954,2487,1432,2394,1648,1792,2477,1905,2115,2011,2309,1866,2255,1590,1851,2620,2055,1658,1635,1625,2580,2083,1903,1789,2180,2257,2085,2448,1899,1830,2178,2714,1872,1709,1654,2021,2777,2258,1726,1372,2418,2995,1578,2089,2045,2012,2508,2001,2010,2511,2078,2024,1841,1781,2599,2089,1878,2075,2129,1826,2095,2616,2029,1985,1943,1688,2668,2040,1815,2219,1933,1657,2479,2125,2356,1862,1415,2407,2218,2504,1406,2176,2244,2377,1683,1523,2700,2171,1569,1933,2246,1920,2192,1682,2354,2114,1877,2130,1816,2255,2359,1821,1507,2285,2097,2123,1786,1835,2452,2020,1973,1701,2070,2271,1802,1912,2351,1634,2157,2015,1834,2417,1512,1670,2330,2373,2026,1657,1645,2535,2093,1552,2088,2263,1883,1824,2170,2009,2007,1836,1974,2542,1419,1985,2358,2011,2293,1754,2052,1962,2358,1773,1839,2221,2089,1919,2074,2107,2014,2173,1703,2115,1898,2112,2570,1750,1859,2145,2136,2379,1884,1884,2090,2600,1794,1693,2310,2080,2218,1612,1957,2525,2070,1850,2114,2034,2140,1783,2439,2528,1469,1772,2129,2452,1823,2085,2289,1859,1913,2296,2304,1903,2007,2027,2183,2073,2195,2010,2365,1633,1923,2671,2050,2101,1785,2217,2181,1992,2031,1964,2293,1924,2054,2265,2125,1704,2044,2371,2129,2407,1439,1944,2444,2239,1814,1959,2216,2249,2023,1684,2286,2253,2084,1803,2371,1956,1833,2329,2052,2170,2011,2000,2299,1861,2091,2237,2121,1799,2115,2344,1879,2026,2098,2381,1749,1768,2273,2249,2041,1924,1979,2237,1875,2024,2342,1897,1877,2169,2205,1738,1965,2353,2184,1667,1936,2079,2191,2047,1942,2086,1977,2016,1801,2093,2363,2232,1797,1774,2077,2191,2220,1970,1727,2121,1760,2180,2430,1604,2133,1929,2032,2220,1987,1972,1905,1989,2207,2026,1872,2337,1813,1970,2102,1961,2181,2000,1881,1816,2063,2246,1982,1656,2378,2219,1656,1956,2279,2023,1819,2194,1939,2008,1869,2187,2218,1780,2254,1822,1994,2187,2127,2139,1758,1987,2093,2171,2025,1836,2200,1938,1995,2152,1844,2165,2070,1980,1885,2099,2122,2206,1807,2078,2176,1708,2323,2082,1978,1874,1996,2100,2096,2106,1963,2018,2094,2005,1904,2227,1961,1913,2253,1915,2020,1986,2155,2015,2022,1938,1920,2357,1936,1917,2143,2008,2043,2098,1919,2047,2151,2143,1885,2041,2034,2109,2138,1974,1883,2207,2012,2128,2019,2069,2027,1885,2397,1855,2069,1976,2422,1687,1975,2414,1873,2084,2130,1951,1980,2242,1977,2181,2008,2036,2102,2122,1894,2173,2093,2043,1894,2344,2029,1713,2346,2049,2018,2016,2135,2074,2075,1930,2031,2176,2187,1896,1959,2215,2017,1888,2366,2042,1838,2108,2212,1986,2078,2014,2004,2230,1837,2294,1927,2094,2146,1962,2136,1964,2082,2131,2075,2042,1861,2210,2131,2013,1943,2092,2290,1780,2052,2093,2165,1929,1999,2251,1866,2005,2266,1970,1964,2183,1986,1883,2217,2137,1893,2054,2141,2104,1959,2013,2160,1948,2108,1996,2114,2121,1807,2284,2046,1929,2095,2136,2119,1907,2097,2180,1911,2104,2115,1915,2076,2237,1933,1851,2351,2108,1841,2103,2128,1945,2129,2147,1769,2276,2007,1935,2213,1984,2085,2033,2005,2067,2151,1907,2144,2084,1956,2090,2108,2022,2056,2064,2096,1932,2068,2201,1940,2091,2069,2025,1994,2075,2233,1899,1920,2244,1944,2080,2063,2039,2050,1971,2124,2083,1911,2157,2154,1838,2067,2139,2062,1872,2121,2173,1949,2045,1954,2233,1941,1969,2221,1895,2038,2139,1986,2036,2112,1966,2074,2040,2077,2083,2038,1890,2128,2147,1977,1969,2097,2101,1807,2244,2154,1869,2045,2110,1987,2065,2031,2057,2029,1941,2117,1997,2046,2082,1988,2052,2057,1985,2048,2066,2081,1991,1955,2069,2066,2123,1897,2041,2099,1995,2038,2069,2008,2017,2102,1951,2029,2144,2035,1922,2131,1971,2066,2018,2069,1976,2060,2166,1855,2089,2118,2013,2006,2057,1998,2101,2033,1995,2053,2098,1961,2051,2040,2045,2096,2005,1954,2099,2152,1878,2096,2059,2027,2022,2074,2061,2002,2069,2047,2042,1970,2116,2071,1999,2006,2076,2000,2104,2114,1859,2193,1985,1980,2149,2002,2038,1998,2077,2061,1997,2100,2036,2051,1966,2060,2159,1968,2031,2078,2075,1892,2153,2090,2002,2046,2044,2066,1994,2084,2055,2033,2027,2080,1948,2142,2112,1977,2075,2013,2038,2058,2172,1943,2017,2126,2008,2131,1934,2106,2071,2009,2029,2054,2096,2012,2046,2028,2126,1953,2045,2138,2049,1918,2107,2073,1960,2196,1925,2066,2087,1918,2197,1994,2054,2051,2037,2130,1897,2120,2093,1986,2042,2095,1999,2029,2112,2036,2020,2052,2018,2062,2132,2014,1995,2078,2059,2025,2047,2093,2022,2057,2124,1879,2202,2053,1964,2134,2045,1980,2060,2171,1949,2093,1964,2141,2018,2007,2121,1940,2142,1968,2017,2117,2110,1881,2137,2077,1911,2209,1998,1974,2124,2069,1947,2117,2101,2000,1977,2200,1957,2040,2120,1988,2090,2039,2111,1993,2041,2127,2009,2032,2081,1964,2177,2027,2087,1925,2147,2103,1907,2179,1991,2074,2067,2005,2054,2140,2009,2011,2108,2061,1941,2166,2051,1981,2094,2068,2020,2043,2143,2027,2005,2070,2029,2090,2082,2038,2037,2005,2162,1977,2067,2122,1978,2061,2057,2070,2001,2126,2092,1963,2051,2040,2119,2011,2094,2024,2001,2053,2082,2107,1986,2007,2128,2025,2013,2084,1979,2121,2065,1989,2031,2124,2029,2036,2024,2080,1997,2087,2095,1951,2038,2164,1966,2026,2140,2016,2053,1974,2116,2029,2036,2096,2029,2019,2098,1945,2123,2095,1986,2033,2122,1965,2057,2072,2032,2124,1947,2091,1961,2175,2020,1965,2102,2059,2033,2049,2062,2034,2007,2074,2028,2056,2107,2003,2033,2055,2095,1992,2092,2048,1972,2094,2084,1961,2120,1996,2067,2075,1995,2052,2055,2115,1942,2096,2047,2017,1999,2112,2090,1966,2105,1975,2074,2093,2030,2002,2056,2073,1953,2151,2065,1970,2036,2042,2080,2055,2004}
};

/**
 * struct used to store a GPIO and a GPIO_pin
 * added by Hermann
 */
typedef struct {
	GPIO_TypeDef * GPIO;
	uint16_t pin;
}LED_GPIO;

// Debugging stuff
uint64_t debugLED_counter_6 = 0, debugLED_counter_5 = 0, debugLED_counter_4 = 0, debugLED_counter_3 = 0, lcd_timer = 0;

// ==========================================================================================
// ============================ Function Declarations =======================================
// ==========================================================================================

void startUpConfigs(void); 			// Function to run the start up configurations.
void delay_ms(uint32_t milli);
void error_(void);					// function to flash the on-board LEDs when an error occurs
void lcd_flush_write(uint8_t row_num, char* msg);
void updateLED(uint8_t pin, bool On, uint8_t type); // implemented in UIUdate
LED_GPIO getGPIO(uint8_t pin, uint8_t type); // implemented in UIUdate
void loadFromEeprom(void);
void saveToEeprom(void);
bool isChannelEmpty(uint8_t index);
void lcd_write(uint8_t col_num, uint8_t row_num, char* msg);
void select_beat(void);

// ==========================================================================================
// ============================ Function Implementations =====================================
// ==========================================================================================

/**
 * Run start up configurations for the STM080. These include
 * > Interrupt configurations
 * > GPIO inputs and outputs
 * > EPROM and On-Board audio interface configutations
 */
void startUpConfigs(){

	// config for LCD
	TM_HD44780_Init(LCD_COLUMNS, LCD_ROWS);
	TM_HD44780_Clear();

	//	turn on reset LED
	GPIO_SetBits(GPIOB, GPIO_PIN_5);
	lcd_write(4, 0, "WELCOME");
	lcd_write(5, 1, "ST080");

	// +++++++++++++++++ configure output pins ++++++++++++++++++++++++

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	/* Enable the clock for all ports*/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;

	/* Configure Button pin as output*/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	
	// pins A7, A8, A10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_10;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// Pins B6-9 and B4 and B5
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// Pins C0-11
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	// Pins E0-3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	// turn on reset LED
	GPIO_SetBits(GPIOB, GPIO_PIN_5);

	// +++++++++++++++++ configure input pins ++++++++++++++++++++++++

	/* Configure Button pin as input */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;		// Input mode
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		// Push - Pull
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;		// To detect a logic high

	// D4 D6 D7
	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_4 | GPIO_PIN_6 | GPIO_PIN_7;;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	// Pins A0-3, 5
	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_5;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// E8-15
	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_Pin_15;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	// +++++++++++++++++ configure interrupt pins ++++++++++++++++++++++++

	// D0-3, E4-7
	bool fail = false;
	if (TM_EXTI_Attach(GPIOD, GPIO_Pin_0, TM_EXTI_Trigger_Rising) != TM_EXTI_Result_Ok){
		// PD0
		fail = true;
	} else if (TM_EXTI_Attach(GPIOD, GPIO_Pin_1, TM_EXTI_Trigger_Rising) != TM_EXTI_Result_Ok){
		// PD1
		fail = true;
	} else if (TM_EXTI_Attach(GPIOD, GPIO_Pin_2, TM_EXTI_Trigger_Rising) != TM_EXTI_Result_Ok){
		// PD2
		fail = true;
	} else if (TM_EXTI_Attach(GPIOD, GPIO_Pin_3, TM_EXTI_Trigger_Rising) != TM_EXTI_Result_Ok){
		// PD3
		fail = true;
	} else if (TM_EXTI_Attach(GPIOE, GPIO_Pin_4, TM_EXTI_Trigger_Rising) != TM_EXTI_Result_Ok){
		// PE4
		fail = true;
	} else if (TM_EXTI_Attach(GPIOE, GPIO_Pin_5, TM_EXTI_Trigger_Rising) != TM_EXTI_Result_Ok){
		// PE5
		fail = true;
	} else if (TM_EXTI_Attach(GPIOE, GPIO_Pin_6, TM_EXTI_Trigger_Rising) != TM_EXTI_Result_Ok){
		// PE6
		fail = true;
	} else if (TM_EXTI_Attach(GPIOE, GPIO_Pin_7, TM_EXTI_Trigger_Rising) != TM_EXTI_Result_Ok){
		// PA7
		fail = true;
	}
	if (fail){
		// failed initialising interrupts.
		error_();
	}

	// Configure Eeprom
	EEPROM_Configuration();
	loadFromEeprom();		// Load the channel rack from the eeprom

	// config for Tempo
	Tempo_Configuration();

	// ask user to select song to edit
	select_beat();

	//Turn off reset LED
	GPIO_ResetBits(GPIOB, GPIO_PIN_5);

	// initialise debugging LEDs

	/* Initialize LEDs */
//	STM_EVAL_LEDInit(LED3);
//	STM_EVAL_LEDInit(LED4);
//	STM_EVAL_LEDInit(LED5);
//	STM_EVAL_LEDInit(LED6);

	/* Turn off LEDs */
//	STM_EVAL_LEDOff(LED3);
//	STM_EVAL_LEDOff(LED4);
//	STM_EVAL_LEDOff(LED5);
//	STM_EVAL_LEDOff(LED6);

}

/**
 * IRQ Handler for:
 * >> instrument buttons (line 0-3)
 * >> mode select (line 4-6)
 * >> save button (line 7)
 *
 */
void TM_EXTI_Handler(uint16_t GPIO_Pin) {

	// +++ debouncing logic (50 milliseconds) +++
	current = tickTime;
	if ((previous - current) < 50) {
		previous = current;
		return;
	}

	/* Handle external line 0 interrupts */
	// button for instrument 1
	if (GPIO_Pin == GPIO_Pin_0) {
		if (MODE == COMPOSER){
			// Change the instrument on the channel rack to the first one
			current_sample = INSTR_1;
			UPDATE_LCD = true;
		}
		else if (MODE == FREESTYLE) {
			PAD_STATE[0] = true;
			STATE_CHANGED = true;
			// Button for the first instrument has been pressed in FREESTYLE mode. Do something.
			played_inst = played_inst + 2;
		}
	}

	/* Handle external line 1 interrupts */
	// button for instrument 2
	else if (GPIO_Pin == GPIO_Pin_1) {
		if (MODE == COMPOSER){
			// change the instrument on the channel rack to the second one
			current_sample = INSTR_2;
			//update the instrument being edited on LCD
			UPDATE_LCD = true;
		}
		else if (MODE == FREESTYLE) {
			PAD_STATE[1] = true;
			STATE_CHANGED = true;
			// Button for the second instrument has been pressed in FREESTYLE mode. Do something.
			played_inst = played_inst + 4;
		}
	}

	/* Handle external line 2 interrupts */
	// button for instrument 3
	else if (GPIO_Pin == GPIO_Pin_2) {
		if (MODE == COMPOSER){
			// change the instrument on the channel rack to the third one
			current_sample = INSTR_3;
			UPDATE_LCD = true;
		}
		else if (MODE == FREESTYLE) {
			PAD_STATE[2] = true;
			STATE_CHANGED = true;
			// Button for the third instrument has been pressed in FREESTYLE mode. Do something.
			played_inst = played_inst + 8;
		}
	}

	/* Handle external line 3 interrupts */
	// button for instrument 4
	else if (GPIO_Pin == GPIO_Pin_3) {
		if (MODE == COMPOSER){
			// change the instrument on the channel rack to the forth one
			current_sample = INSTR_4;
			UPDATE_LCD = true;
		}
		else if (MODE == FREESTYLE) {
			PAD_STATE[3] = true;
			STATE_CHANGED = true;
			// Button for the forth instrument has been pressed in FREESTYLE mode. Do something.
			played_inst = played_inst + 16;
		}
	}

	/* Handle external line 4 interrupts */
	// switch to COMPOSER MODE
	else if (GPIO_Pin == GPIO_Pin_4) {
		MODE = COMPOSER;
		status = true;
		resetLEDs = true;
//		update the Mode on LCD
		UPDATE_LCD = true;
	}

	/* Handle external line 5 interrupts */
	// switch to PLAYBACK MODE
	else if (GPIO_Pin == GPIO_Pin_5) {
		MODE = PLAYBACK;
		resetLEDs = true;
//		update the Mode on LCD
		UPDATE_LCD = true;
	}

	/* Handle external line 6 interrupts */
	// switch to FREESTYLE MODE
	else if (GPIO_Pin == GPIO_Pin_6) {
//		update the Mode on LCD
		UPDATE_LCD = true;
		MODE = FREESTYLE;
		resetLEDs = true;
	}

	/* Handle external line 7 interrupts */
	// Save pin
	else if (GPIO_Pin == GPIO_Pin_7) {
		if (MODE == COMPOSER || MODE == PLAYBACK) {
			MODE = SAVE;
			status = true;
			resetLEDs = true;
	//		update the Mode on LCD
			UPDATE_LCD = true;
		}
		// Save the channelRack Array to the EEPROM
	}

	previous = current; // for debouncing
}


// Remap the LCD functions
/**
 * Method to flush the LCD screen and write a new message on the specified row
 *
 * @param msg String to write to teh LCD screen
 * @param row_num
 *
 */
void lcd_flush_write(uint8_t row_num, char* msg){
	TM_HD44780_Clear();
	TM_HD44780_Puts(0,row_num,msg);
}

/**
 *
 * @brief Method to add a new message on the specified column row
 * @param msg String to write to teh LCD screen
 * @param col_num
 * @param col_num
 *
 */
void lcd_write(uint8_t col_num, uint8_t row_num, char* msg){
	TM_HD44780_Puts(col_num,row_num,msg);
}

/*
 * 	@brief	Saves the array "channelRack[][][]" to the eeprom, the size of this array is CHANNEL_RACK_SIZE
 */
void saveToEeprom(){
	int i,j,k,l=0;
	uint8_t temp = 0;
	for (i = 0; i < 16; ++i) {
		for (j = 0; j < 4; ++j) {
			for (k = 0; k < 16; ++k, ++l) {
				temp = (uint8_t) channelRack[i][j][k];
				EEPROM_Write(l, temp);
			}
		}
	} // end of for loops
}// end of saveToEeprom

/*
 * @brief	Initializes the "channelRack[][][]" using data read from the eeprom
 */
void loadFromEeprom(){
	int i, j, k, l = 0;
	bool temp = false;
	for (i = 0; i < 16; ++i) {
		for (j = 0; j < 4; ++j) {
			for (k = 0; k < 16; ++k, ++l) {
				temp = (bool) EEPROM_Read(l);
				channelRack[i][j][k] = temp;
			}
		}
	} // end of for loops
} // end of loadFromEeprom

/**
 * @brief	Check if a channel is empty
 */
bool isChannelEmpty(uint8_t index){
	int j,k=0;
	bool empty = true;
	for (j=0;j<4;++j){
		for (k=0;k<16;++k){
			if(channelRack[4*16*index][j][k]!=0){
				empty = false;
				return empty;
			}
		}
	}
	return empty;
}


/**
  * @brief  Delay in ms
  * @param  integer amount of milliseconds
  * @retval None
  */
void delay_ms(uint32_t milli)
{
	uint32_t delay = milli * 17612;              // approximate loops per ms at 168 MHz, Debug config
	for(; delay != 0; delay--);
}

void select_beat(){
	lcd_flush_write(1, "Select a song!");
	lcd_write(5, 0, "ST080");
	MODE=ENTER;
}

/**
 * Function to flash the LEDs when an error occurs
 */
void error_(void)
{
	MODE = ERROR_MODE;

	/* Initialize LEDs */
	STM_EVAL_LEDInit(LED3);
	STM_EVAL_LEDInit(LED4);
	STM_EVAL_LEDInit(LED5);
	STM_EVAL_LEDInit(LED6);

	/* Turn off some LEDs */
	STM_EVAL_LEDOff(LED3);
	STM_EVAL_LEDOff(LED4);
	/* Turn on some LEDs */
	STM_EVAL_LEDOn(LED5);
	STM_EVAL_LEDOn(LED6);

	lcd_flush_write(0, "ERROR! Reset device");

	GPIO_SetBits(GPIOB, 6);
	GPIO_SetBits(GPIOB, 7);
	GPIO_SetBits(GPIOB, 8);
	GPIO_SetBits(GPIOB, 9);

	while(1){
		delay_ms(500);
		STM_EVAL_LEDToggle(LED3);
		STM_EVAL_LEDToggle(LED4);
		STM_EVAL_LEDToggle(LED5);
		STM_EVAL_LEDToggle(LED6);
	}
}
#endif /* UTILS080_H_ */
