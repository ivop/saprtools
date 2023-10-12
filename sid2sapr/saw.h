#ifndef GENERATE

#pragma once

struct tabitem {
    int freq;
    int div1;
    int div3;
} sawtab_full[] = {
    {    56.93, 0xac, 0xad },
    {    57.58, 0xab, 0xac },
    {    58.24, 0xaa, 0xab },
    {    58.91, 0xa9, 0xaa },
    {    59.60, 0xa8, 0xa9 },
    {    60.30, 0xa7, 0xa8 },
    {    61.01, 0xa6, 0xa7 },
    {    61.73, 0xa5, 0xa6 },
    {    62.46, 0xa4, 0xa5 },
    {    63.21, 0xa3, 0xa4 },
    {    63.97, 0xa2, 0xa3 },
    {    64.75, 0xa1, 0xa2 },
    {    65.54, 0xa0, 0xa1 },
    {    66.34, 0x9f, 0xa0 },
    {    67.16, 0x9e, 0x9f },
    {    68.00, 0x9d, 0x9e },
    {    68.84, 0x9c, 0x9d },
    {    69.71, 0x9b, 0x9c },
    {    70.59, 0x9a, 0x9b },
    {    71.49, 0x99, 0x9a },
    {    72.41, 0x98, 0x99 },
    {    73.34, 0x97, 0x98 },
    {    74.30, 0x96, 0x97 },
    {    75.27, 0x95, 0x96 },
    {    76.26, 0x94, 0x95 },
    {    77.27, 0x93, 0x94 },
    {    78.30, 0x92, 0x93 },
    {    79.35, 0x91, 0x92 },
    {    80.42, 0x90, 0x91 },
    {    81.52, 0x8f, 0x90 },
    {    82.63, 0x8e, 0x8f },
    {    83.77, 0x8d, 0x8e },
    {    84.94, 0x8c, 0x8d },
    {    86.12, 0x8b, 0x8c },
    {    87.34, 0x8a, 0x8b },
    {    88.57, 0x89, 0x8a },
    {    89.84, 0x88, 0x89 },
    {    91.13, 0x87, 0x88 },
    {    92.45, 0x86, 0x87 },
    {    93.80, 0x85, 0x86 },
    {    95.18, 0x84, 0x85 },
    {    96.59, 0x83, 0x84 },
    {    98.03, 0x82, 0x83 },
    {    99.51, 0x81, 0x82 },
    {   101.02, 0x80, 0x81 },
    {   102.56, 0x7f, 0x80 },
    {   104.14, 0x7e, 0x7f },
    {   105.75, 0x7d, 0x7e },
    {   107.40, 0x7c, 0x7d },
    {   109.09, 0x7b, 0x7c },
    {   110.83, 0x7a, 0x7b },
    {   112.60, 0x79, 0x7a },
    {   113.22, 0xac, 0xae },
    {   114.42, 0x78, 0x79 },
    {   114.51, 0xab, 0xad },
    {   115.82, 0xaa, 0xac },
    {   116.28, 0x77, 0x78 },
    {   117.16, 0xa9, 0xab },
    {   118.18, 0x76, 0x77 },
    {   118.51, 0xa8, 0xaa },
    {   119.90, 0xa7, 0xa9 },
    {   120.14, 0x75, 0x76 },
    {   121.30, 0xa6, 0xa8 },
    {   122.14, 0x74, 0x75 },
    {   122.73, 0xa5, 0xa7 },
    {   124.19, 0x73, 0x74 },
    {   124.19, 0xa4, 0xa6 },
    {   125.67, 0xa3, 0xa5 },
    {   126.30, 0x72, 0x73 },
    {   127.18, 0xa2, 0xa4 },
    {   128.45, 0x71, 0x72 },
    {   128.72, 0xa1, 0xa3 },
    {   130.29, 0xa0, 0xa2 },
    {   130.67, 0x70, 0x71 },
    {   131.88, 0x9f, 0xa1 },
    {   132.94, 0x6f, 0x70 },
    {   133.50, 0x9e, 0xa0 },
    {   135.16, 0x9d, 0x9f },
    {   135.27, 0x6e, 0x6f },
    {   136.84, 0x9c, 0x9e },
    {   137.67, 0x6d, 0x6e },
    {   138.56, 0x9b, 0x9d },
    {   140.13, 0x6c, 0x6d },
    {   140.30, 0x9a, 0x9c },
    {   142.09, 0x99, 0x9b },
    {   142.65, 0x6b, 0x6c },
    {   143.90, 0x98, 0x9a },
    {   145.25, 0x6a, 0x6b },
    {   145.75, 0x97, 0x99 },
    {   147.64, 0x96, 0x98 },
    {   147.91, 0x69, 0x6a },
    {   149.56, 0x95, 0x97 },
    {   150.65, 0x68, 0x69 },
    {   151.52, 0x94, 0x96 },
    {   153.47, 0x67, 0x68 },
    {   153.53, 0x93, 0x95 },
    {   155.57, 0x92, 0x94 },
    {   156.36, 0x66, 0x67 },
    {   157.65, 0x91, 0x93 },
    {   159.34, 0x65, 0x66 },
    {   159.77, 0x90, 0x92 },
    {   161.94, 0x8f, 0x91 },
    {   162.40, 0x64, 0x65 },
    {   164.15, 0x8e, 0x90 },
    {   165.56, 0x63, 0x64 },
    {   166.40, 0x8d, 0x8f },
    {   168.71, 0x8c, 0x8e },
    {   168.80, 0x62, 0x63 },
    {   171.06, 0x8b, 0x8d },
    {   172.15, 0x61, 0x62 },
    {   172.75, 0xaa, 0xad },
    {   173.46, 0x8a, 0x8c },
    {   175.59, 0x60, 0x61 },
    {   175.91, 0x89, 0x8b },
    {   178.42, 0x88, 0x8a },
    {   178.81, 0xa7, 0xaa },
    {   179.14, 0x5f, 0x60 },
    {   180.97, 0x87, 0x89 },
    {   182.79, 0x5e, 0x5f },
    {   183.59, 0x86, 0x88 },
    {   185.20, 0xa4, 0xa7 },
    {   186.26, 0x85, 0x87 },
    {   186.56, 0x5d, 0x5e },
    {   188.99, 0x84, 0x86 },
    {   190.45, 0x5c, 0x5d },
    {   191.78, 0x83, 0x85 },
    {   191.93, 0xa1, 0xa4 },
    {   194.46, 0x5b, 0x5c },
    {   194.63, 0x82, 0x84 },
    {   197.54, 0x81, 0x83 },
    {   198.59, 0x5a, 0x5b },
    {   199.04, 0x9e, 0xa1 },
    {   200.53, 0x80, 0x82 },
    {   202.87, 0x59, 0x5a },
    {   203.58, 0x7f, 0x81 },
    {   206.55, 0x9b, 0x9e },
    {   206.70, 0x7e, 0x80 },
    {   207.28, 0x58, 0x59 },
    {   209.89, 0x7d, 0x7f },
    {   211.83, 0x57, 0x58 },
    {   213.15, 0x7c, 0x7e },
    {   214.50, 0x98, 0x9b },
    {   216.50, 0x7b, 0x7d },
    {   216.54, 0x56, 0x57 },
    {   219.92, 0x7a, 0x7c },
    {   221.40, 0x55, 0x56 },
    {   222.91, 0x95, 0x98 },
    {   223.43, 0x79, 0x7b },
    {   226.44, 0x54, 0x55 },
    {   227.02, 0x78, 0x7a },
    {   230.69, 0x77, 0x79 },
    {   231.64, 0x53, 0x54 },
    {   231.82, 0x92, 0x95 },
    {   234.33, 0xa8, 0xac },
    {   234.46, 0x76, 0x78 },
    {   237.03, 0x52, 0x53 },
    {   238.32, 0x75, 0x77 },
    {   241.29, 0x8f, 0x92 },
    {   242.27, 0x74, 0x76 },
    {   242.61, 0x51, 0x52 },
    {   245.49, 0xa4, 0xa8 },
    {   246.33, 0x73, 0x75 },
    {   248.38, 0x50, 0x51 },
    {   250.49, 0x72, 0x74 },
    {   251.34, 0x8c, 0x8f },
    {   254.37, 0x4f, 0x50 },
    {   254.75, 0x71, 0x73 },
    {   257.47, 0xa0, 0xa4 },
    {   259.12, 0x70, 0x72 },
    {   260.57, 0x4e, 0x4f },
    {   262.03, 0x89, 0x8c },
    {   263.61, 0x6f, 0x71 },
    {   267.00, 0x4d, 0x4e },
    {   268.22, 0x6e, 0x70 },
    {   270.34, 0x9c, 0xa0 },
    {   272.94, 0x6d, 0x6f },
    {   273.43, 0x86, 0x89 },
    {   273.68, 0x4c, 0x4d },
    {   277.80, 0x6c, 0x6e },
    {   280.61, 0x4b, 0x4c },
    {   282.78, 0x6b, 0x6d },
    {   284.21, 0x98, 0x9c },
    {   285.58, 0x83, 0x86 },
    {   287.80, 0x4a, 0x4b },
    {   287.90, 0x6a, 0x6c },
    {   293.16, 0x69, 0x6b },
    {   295.28, 0x49, 0x4a },
    {   298.06, 0xa6, 0xab },
    {   298.56, 0x68, 0x6a },
    {   298.56, 0x80, 0x83 },
    {   299.16, 0x94, 0x98 },
    {   303.05, 0x48, 0x49 },
    {   304.12, 0x67, 0x69 },
    {   309.83, 0x66, 0x68 },
    {   311.13, 0x47, 0x48 },
    {   312.45, 0x7d, 0x80 },
    {   315.34, 0x90, 0x94 },
    {   315.70, 0x65, 0x67 },
    {   316.12, 0xa1, 0xa6 },
    {   319.54, 0x46, 0x47 },
    {   321.74, 0x64, 0x66 },
    {   327.33, 0x7a, 0x7d },
    {   327.96, 0x63, 0x65 },
    {   328.29, 0x45, 0x46 },
    {   332.85, 0x8c, 0x90 },
    {   334.36, 0x62, 0x64 },
    {   335.88, 0x9c, 0xa1 },
    {   337.41, 0x44, 0x45 },
    {   339.74, 0xaa, 0xb0 },
    {   340.95, 0x61, 0x63 },
    {   343.29, 0x77, 0x7a },
    {   346.92, 0x43, 0x44 },
    {   347.73, 0x60, 0x62 },
    {   351.87, 0x88, 0x8c },
    {   354.72, 0x5f, 0x61 },
    {   356.83, 0x42, 0x43 },
    {   357.55, 0x97, 0x9c },
    {   360.46, 0x74, 0x77 },
    {   361.93, 0x5e, 0x60 },
    {   364.01, 0xa4, 0xaa },
    {   367.17, 0x41, 0x42 },
    {   369.35, 0x5d, 0x5f },
    {   372.57, 0x84, 0x88 },
    {   377.01, 0x5c, 0x5e },
    {   377.97, 0x40, 0x41 },
    {   378.94, 0x71, 0x74 },
    {   381.39, 0x92, 0x97 },
    {   384.90, 0x5b, 0x5d },
    {   389.26, 0x3f, 0x40 },
    {   390.97, 0x9e, 0xa4 },
    {   393.05, 0x5a, 0x5c },
    {   395.15, 0x80, 0x84 },
    {   398.89, 0x6e, 0x71 },
    {   401.05, 0x3e, 0x3f },
    {   401.46, 0x59, 0x5b },
    {   407.69, 0x8d, 0x92 },
    {   410.14, 0x58, 0x5a },
    {   413.39, 0x3d, 0x3e },
    {   419.11, 0x57, 0x59 },
    {   419.85, 0x7c, 0x80 },
    {   420.45, 0x6b, 0x6e },
    {   421.05, 0x98, 0x9e },
    {   422.25, 0xa4, 0xab },
    {   426.31, 0x3c, 0x3d },
    {   428.37, 0x56, 0x58 },
    {   436.81, 0x88, 0x8d },
    {   437.94, 0x55, 0x57 },
    {   439.84, 0x3b, 0x3c },
    {   443.81, 0x68, 0x6b },
    {   446.94, 0x78, 0x7c },
    {   447.84, 0x54, 0x56 },
    {   454.03, 0x3a, 0x3b },
    {   454.73, 0x92, 0x98 },
    {   458.08, 0x53, 0x55 },
    {   458.97, 0x9d, 0xa4 },
    {   468.67, 0x52, 0x54 },
    {   468.92, 0x39, 0x3a },
    {   469.17, 0x65, 0x68 },
    {   469.17, 0x83, 0x88 },
    {   476.73, 0x74, 0x78 },
    {   479.63, 0x51, 0x53 },
    {   479.83, 0xa4, 0xac },
    {   484.55, 0x38, 0x39 },
    {   490.99, 0x50, 0x52 },
    {   492.62, 0x8c, 0x92 },
    {   496.76, 0x62, 0x65 },
    {   500.69, 0x96, 0x9d },
    {   500.97, 0x37, 0x38 },
    {   502.75, 0x4f, 0x51 },
    {   505.26, 0x7e, 0x83 },
    {   509.61, 0x70, 0x74 },
    {   514.94, 0x4e, 0x50 },
    {   518.25, 0x36, 0x37 },
    {   518.55, 0xa7, 0xb0 },
    {   526.87, 0x5f, 0x62 },
    {   527.58, 0x4d, 0x4f },
    {   527.81, 0x9c, 0xa4 },
    {   535.46, 0x86, 0x8c },
    {   536.43, 0x35, 0x36 },
    {   540.69, 0x4c, 0x4e },
    {   545.68, 0x79, 0x7e },
    {   546.01, 0x6c, 0x70 },
    {   548.38, 0x8f, 0x96 },
    {   554.29, 0x4b, 0x4d },
    {   555.59, 0x34, 0x35 },
    {   559.80, 0x5c, 0x5f },
    {   568.41, 0x4a, 0x4c },
    {   575.79, 0x33, 0x34 },
    {   576.17, 0x9e, 0xa7 },
    {   579.56, 0xa6, 0xb0 },
    {   583.08, 0x49, 0x4b },
    {   583.37, 0x94, 0x9c },
    {   584.14, 0x80, 0x86 },
    {   586.46, 0x68, 0x6c },
    {   591.15, 0x74, 0x79 },
    {   595.92, 0x59, 0x5c },
    {   597.12, 0x32, 0x33 },
    {   598.33, 0x48, 0x4a },
    {   603.21, 0x88, 0x8f },
    {   614.18, 0x47, 0x49 },
    {   619.65, 0x31, 0x32 },
    {   630.67, 0x46, 0x48 },
    {   631.57, 0x64, 0x68 },
    {   635.64, 0x56, 0x59 },
    {   639.77, 0x7a, 0x80 },
    {   642.55, 0x6f, 0x74 },
    {   643.49, 0x30, 0x31 },
    {   643.95, 0x95, 0x9e },
    {   647.83, 0x45, 0x47 },
    {   648.19, 0x8c, 0x94 },
    {   652.00, 0x9c, 0xa6 },
    {   665.71, 0x44, 0x46 },
    {   666.71, 0x81, 0x88 },
    {   668.72, 0x2f, 0x30 },
    {   671.76, 0xa1, 0xac },
    {   679.48, 0x53, 0x56 },
    {   682.10, 0x60, 0x64 },
    {   684.33, 0x43, 0x45 },
    {   695.47, 0x2e, 0x2f },
    {   700.97, 0x6a, 0x6f },
    {   703.75, 0x42, 0x44 },
    {   703.75, 0x74, 0x7a },
    {   703.75, 0xa4, 0xb0 },
    {   723.86, 0x2d, 0x2e },
    {   724.00, 0x41, 0x43 },
    {   724.45, 0x84, 0x8c },
    {   724.45, 0x8c, 0x95 },
    {   728.02, 0x50, 0x53 },
    {   738.94, 0x5c, 0x60 },
    {   738.94, 0x92, 0x9c },
    {   740.79, 0x7a, 0x81 },
    {   745.15, 0x40, 0x42 },
    {   749.55, 0xa5, 0xb2 },
    {   754.02, 0x2c, 0x2d },
    {   767.23, 0x3f, 0x41 },
    {   767.73, 0x65, 0x6a },
    {   767.73, 0x96, 0xa1 },
    {   777.83, 0x6e, 0x74 },
    {   781.94, 0x4d, 0x50 },
    {   786.10, 0x2b, 0x2c },
    {   790.31, 0x3e, 0x40 },
    {   803.19, 0x58, 0x5c },
    {   812.02, 0x98, 0xa4 },
    {   812.02, 0xa4, 0xb2 },
    {   814.44, 0x3d, 0x3f },
    {   815.00, 0x7c, 0x84 },
    {   820.28, 0x2a, 0x2b },
    {   821.04, 0x83, 0x8c },
    {   827.94, 0x73, 0x7a },
    {   839.70, 0x3c, 0x3e },
    {   842.09, 0x4a, 0x4d },
    {   844.50, 0x60, 0x65 },
    {   844.50, 0x88, 0x92 },
    {   856.74, 0x29, 0x2a },
    {   864.25, 0x68, 0x6e },
    {   866.15, 0x3b, 0x3d },
    {   874.48, 0x98, 0xa5 },
    {   876.21, 0x54, 0x58 },
    {   885.84, 0x8b, 0x96 },
    {   893.87, 0x3a, 0x3c },
    {   895.68, 0x28, 0x29 },
    {   895.68, 0xa1, 0xb0 },
    {   909.46, 0x47, 0x4a },
    {   922.95, 0x39, 0x3b },
    {   923.67, 0x74, 0x7c },
    {   931.43, 0x6c, 0x73 },
    {   933.39, 0x5b, 0x60 },
    {   937.34, 0x27, 0x28 },
    {   938.33, 0x7a, 0x83 },
    {   947.35, 0x8c, 0x98 },
    {   953.47, 0x38, 0x3a },
    {   959.66, 0x50, 0x54 },
    {   959.66, 0x96, 0xa4 },
    {   965.93, 0x62, 0x68 },
    {   974.42, 0x7e, 0x88 },
    {   981.98, 0x26, 0x27 },
    {   985.25, 0x44, 0x47 },
    {   985.52, 0x37, 0x39 },
    {  1019.22, 0x36, 0x38 },
    {  1029.88, 0x25, 0x26 },
    {  1033.48, 0x80, 0x8b },
    {  1033.48, 0x8b, 0x98 },
    {  1037.10, 0x56, 0x5b },
    {  1054.68, 0x35, 0x37 },
    {  1055.62, 0x4c, 0x50 },
    {  1055.62, 0x65, 0x6c },
    {  1055.62, 0x6c, 0x74 },
    {  1070.92, 0x41, 0x44 },
    {  1074.82, 0x92, 0xa1 },
    {  1081.37, 0x24, 0x25 },
    {  1082.69, 0x71, 0x7a },
    {  1086.67, 0x5c, 0x62 },
    {  1092.02, 0x34, 0x36 },
    {  1119.60, 0x80, 0x8c },
    {  1131.39, 0x33, 0x35 },
    {  1136.82, 0x23, 0x24 },
    {  1136.83, 0x74, 0x7e },
    {  1151.59, 0x88, 0x96 },
    {  1159.12, 0x51, 0x56 },
    {  1166.74, 0x48, 0x4c },
    {  1168.28, 0x3e, 0x41 },
    {  1172.91, 0x32, 0x34 },
    {  1196.66, 0x22, 0x23 },
    {  1206.43, 0x5e, 0x65 },
    {  1216.77, 0x31, 0x33 },
    {  1218.03, 0x64, 0x6c },
    {  1221.38, 0x75, 0x80 },
    {  1231.56, 0x56, 0x5c },
    {  1240.17, 0x7e, 0x8b },
    {  1261.34, 0x21, 0x22 },
    {  1263.14, 0x30, 0x32 },
    {  1263.14, 0x68, 0x71 },
    {  1279.54, 0x3b, 0x3e },
    {  1296.38, 0x44, 0x48 },
    {  1304.01, 0x4c, 0x51 },
    {  1312.21, 0x2f, 0x31 },
    {  1313.66, 0x83, 0x92 },
    {  1331.42, 0x20, 0x21 },
    {  1343.52, 0x6a, 0x74 },
    {  1343.52, 0x74, 0x80 },
    {  1364.19, 0x2e, 0x30 },
    {  1392.03, 0x57, 0x5e },
    {  1407.50, 0x1f, 0x20 },
    {  1407.50, 0x38, 0x3b },
    {  1407.50, 0x50, 0x56 },
    {  1407.50, 0x7a, 0x88 },
    {  1419.33, 0x2d, 0x2f },
    {  1421.03, 0x5c, 0x64 },
    {  1448.89, 0x40, 0x44 },
    {  1465.66, 0x6a, 0x75 },
    {  1477.87, 0x2c, 0x2e },
    {  1477.87, 0x47, 0x4c },
    {  1490.29, 0x1e, 0x1f },
    {  1492.80, 0x5f, 0x68 },
    {  1515.77, 0x71, 0x7e },
    {  1540.12, 0x2b, 0x2d },
    {  1555.66, 0x35, 0x38 },
    {  1580.61, 0x1d, 0x1e },
    {  1606.38, 0x2a, 0x2c },
    {  1612.22, 0x60, 0x6a },
    {  1624.04, 0x4a, 0x50 },
    {  1624.04, 0x50, 0x57 },
    {  1630.01, 0x3c, 0x40 },
    {  1642.08, 0x68, 0x74 },
    {  1642.08, 0x74, 0x83 },
    {  1677.02, 0x29, 0x2b },
    {  1679.40, 0x1c, 0x1d },
    {  1679.40, 0x54, 0x5c },
    {  1689.00, 0x42, 0x47 },
    {  1728.51, 0x32, 0x35 },
    {  1752.42, 0x28, 0x2a },
    {  1759.37, 0x6c, 0x7a },
    {  1787.75, 0x1b, 0x1c },
    {  1791.36, 0x56, 0x5f },
    {  1791.36, 0x5f, 0x6a },
    {  1833.02, 0x27, 0x29 },
    {  1847.34, 0x38, 0x3c },
    {  1894.71, 0x44, 0x4a },
    {  1894.71, 0x64, 0x71 },
    {  1906.93, 0x1a, 0x1b },
    {  1919.31, 0x26, 0x28 },
    {  1919.31, 0x49, 0x50 },
    {  1931.86, 0x2f, 0x32 },
    {  1948.84, 0x3d, 0x42 },
    {  1970.50, 0x56, 0x60 },
    {  2011.85, 0x25, 0x27 },
    {  2015.28, 0x4c, 0x54 },
    {  2038.44, 0x19, 0x1a },
    {  2052.60, 0x5c, 0x68 },
    {  2111.25, 0x24, 0x26 },
    {  2111.25, 0x34, 0x38 },
    {  2111.25, 0x65, 0x74 },
    {  2173.34, 0x2c, 0x2f },
    {  2184.05, 0x18, 0x19 },
    {  2189.44, 0x4d, 0x56 },
    {  2218.20, 0x23, 0x25 },
    {  2239.20, 0x3e, 0x44 },
    {  2239.20, 0x54, 0x5f },
    {  2262.05, 0x5e, 0x6c },
    {  2273.65, 0x38, 0x3d },
    {  2303.18, 0x42, 0x49 },
    {  2333.48, 0x22, 0x24 },
    {  2345.83, 0x17, 0x18 },
    {  2436.05, 0x30, 0x34 },
    {  2436.05, 0x57, 0x64 },
    {  2458.00, 0x21, 0x23 },
    {  2463.12, 0x29, 0x2c },
    {  2463.12, 0x44, 0x4c },
    {  2463.12, 0x4c, 0x56 },
    {  2526.28, 0x16, 0x17 },
    {  2592.76, 0x20, 0x22 },
    {  2639.06, 0x50, 0x5c },
    {  2687.04, 0x33, 0x38 },
    {  2687.04, 0x38, 0x3e },
    {  2728.38, 0x15, 0x16 },
    {  2736.80, 0x44, 0x4d },
    {  2738.91, 0x1f, 0x21 },
    {  2815.00, 0x26, 0x29 },
    {  2815.00, 0x3b, 0x42 },
    {  2815.00, 0x56, 0x65 },
    {  2842.06, 0x2c, 0x30 },
    {  2878.97, 0x49, 0x54 },
    {  2897.79, 0x1e, 0x20 },
    {  2955.74, 0x14, 0x15 },
    {  3016.07, 0x50, 0x5e },
    {  3070.90, 0x1d, 0x1f },
    {  3078.90, 0x3c, 0x44 },
    {  3166.87, 0x42, 0x4c },
    {  3212.77, 0x13, 0x14 },
    {  3224.45, 0x2e, 0x33 },
    {  3248.07, 0x23, 0x26 },
    {  3248.07, 0x4a, 0x57 },
    {  3260.01, 0x1c, 0x1e },
    {  3284.16, 0x32, 0x38 },
    {  3358.80, 0x28, 0x2c },
    {  3467.15, 0x1b, 0x1d },
    {  3504.84, 0x12, 0x13 },
    {  3518.74, 0x34, 0x3b },
    {  3518.74, 0x3b, 0x44 },
    {  3518.74, 0x44, 0x50 },
    {  3694.68, 0x1a, 0x1c },
    {  3789.42, 0x20, 0x23 },
    {  3838.63, 0x11, 0x12 },
    {  3838.63, 0x3e, 0x49 },
    {  3940.99, 0x29, 0x2e },
    {  3940.99, 0x47, 0x56 },
    {  3945.38, 0x19, 0x1b },
    {  3958.59, 0x34, 0x3c },
    {  4030.56, 0x24, 0x28 },
    {  4105.20, 0x2c, 0x32 },
    {  4222.49, 0x10, 0x11 },
    {  4222.49, 0x18, 0x1a },
    {  4222.49, 0x38, 0x42 },
    {  4222.49, 0x42, 0x50 },
    {  4478.40, 0x1d, 0x20 },
    {  4524.10, 0x2d, 0x34 },
    {  4529.88, 0x17, 0x19 },
    {  4547.30, 0x3d, 0x4a },
    {  4666.97, 0x0f, 0x10 },
    {  4691.66, 0x32, 0x3b },
    {  4872.11, 0x16, 0x18 },
    {  4926.24, 0x20, 0x24 },
    {  4926.24, 0x24, 0x29 },
    {  4926.24, 0x38, 0x44 },
    {  5185.52, 0x0e, 0x0f },
    {  5254.66, 0x15, 0x17 },
    {  5278.12, 0x26, 0x2c },
    {  5278.12, 0x2c, 0x34 },
    {  5374.08, 0x1a, 0x1d },
    {  5374.08, 0x33, 0x3e },
    {  5684.12, 0x14, 0x16 },
    {  5795.58, 0x0d, 0x0e },
    {  5911.49, 0x2e, 0x38 },
    {  5911.49, 0x38, 0x47 },
    {  6032.13, 0x26, 0x2d },
    {  6157.80, 0x1c, 0x20 },
    {  6168.51, 0x13, 0x15 },
    {  6333.74, 0x1f, 0x24 },
    {  6333.74, 0x34, 0x42 },
    {  6520.03, 0x0c, 0x0d },
    {  6568.32, 0x17, 0x1a },
    {  6568.32, 0x29, 0x32 },
    {  6717.60, 0x12, 0x14 },
    {  6820.95, 0x30, 0x3d },
    {  7037.49, 0x20, 0x26 },
    {  7343.47, 0x11, 0x13 },
    {  7389.36, 0x0b, 0x0c },
    {  7389.36, 0x24, 0x2c },
    {  7389.36, 0x2c, 0x38 },
    {  7917.17, 0x18, 0x1c },
    {  8061.12, 0x10, 0x12 },
    {  8061.12, 0x28, 0x33 },
    {  8210.40, 0x14, 0x17 },
    {  8444.99, 0x0a, 0x0b },
    {  8444.99, 0x1a, 0x1f },
    {  8444.99, 0x1f, 0x26 },
    {  8867.24, 0x24, 0x2e },
    {  8889.46, 0x0f, 0x11 },
    {  9744.21, 0x09, 0x0a },
    {  9852.48, 0x0e, 0x10 },
    {  9852.48, 0x1a, 0x20 },
    {  9852.48, 0x20, 0x29 },
    {  9852.48, 0x29, 0x38 },
    { 10556.23, 0x11, 0x14 },
    { 10556.23, 0x14, 0x18 },
    { 10556.23, 0x26, 0x34 },
    { 10981.10, 0x0d, 0x0f },
    { 11084.04, 0x1c, 0x24 },
    { 11368.25, 0x08, 0x09 },
    { 11368.25, 0x23, 0x30 },
    { 11822.98, 0x15, 0x1a },
    { 12315.60, 0x0c, 0x0e },
    { 12315.60, 0x20, 0x2c },
    { 12667.48, 0x18, 0x1f },
    { 13435.20, 0x07, 0x08 },
    { 13435.20, 0x1d, 0x28 },
    { 13909.39, 0x0b, 0x0d },
    { 14074.98, 0x0e, 0x11 },
    { 14778.72, 0x14, 0x1a },
    { 14778.72, 0x1a, 0x24 },
    { 14778.73, 0x10, 0x14 },
    { 15834.35, 0x0a, 0x0c },
    { 16122.25, 0x06, 0x07 },
    { 16420.81, 0x17, 0x20 },
    { 17734.47, 0x10, 0x15 },
    { 18189.20, 0x09, 0x0b },
    { 18473.41, 0x14, 0x1c },
    { 19704.97, 0x05, 0x06 },
    { 19704.97, 0x0b, 0x0e },
    { 19704.97, 0x1a, 0x29 },
};

struct tabitem *sawtab = sawtab_full;
int sawtabsize = sizeof(sawtab_full)/sizeof(struct tabitem);

#else

// gcc -DGENERATE -o saw saw.c
// ./saw | sort -n | sed 's/^\(.*\)$/    {\1 },/' | uniq > foo.bar

#include <stdio.h>

#define ATARI_CLOCK     1773447.0

int main(int argc, char **argv) {

    // channels 1+3 filter, both @ 1.77MHz
    // sawtooth of f1-f3 Hz, carrier of f1+f3
    //
    // dividers(t) = (x(t), x(t)+q)
    // x(t) = x(0) + t*p
    //
    // stable:
    //                  x(0)=0,  p=1,  q=1
    //                  x(0)=0,  p=2,  q=2
    //                  x(0)=2,  p=3,  q=3
    //                  x(0)=0,  p=4,  q=4
    //                  x(0)=1,  p=5,  q=5
    //                  x(0)=2,  p=6,  q=5
    //                  x(0)=3,  p=7,  q=7
    //                  x(0)=4,  p=8,  q=8
    //                  x(0)=5,  p=9,  q=9
    //                  x(0)=6,  p=10, q=10
    //                  x(0)=7,  p=11, q=11
    //                  x(0)=8,  p=12, q=12
    //                  x(0)=9,  p=13, q=13
    //                  x(0)=10, p=14, q=14
    //                  x(0)=11, p=15, q=15
    //
    // except for:
    //                  x(0)=0, p=1,  q=2       slightly unstabel, noticable?
    // p == q
    //
    // Fsaw = G(x(t)) - G(x(t)+q)
    // Fcar = G(x(t)) + G(x(t)+q)
    //
    // G(y) = 1773447 / (y + 4)

    struct tabitem {
        int x0, p, q;
    } table[] = {
#if 1
        {  0, 1,   2 },
        {  0, 1,   1 },             // slightly unstable
        {  2, 3,   3 },
        {  0, 4,   4 },
        {  0, 2,   2 },
        {  1, 5,   5 },
        {  2, 6,   6 },
        {  3, 7,   7 },
        {  4, 8,   8 },
        {  5, 9,   9 },
        {  6, 10, 10 },
        {  7, 11, 11 },
        {  8, 12, 12 },
        {  9, 13, 13 },
        { 10, 14, 14 },
        { 11, 15, 15 }
#endif
    };

    for (int i=0; i<sizeof(table)/sizeof(struct tabitem); i++) {
        int x0 = table[i].x0;
        int p  = table[i].p;
        int q  = table[i].q;

        for (int t=0; t<256; t++) {
            int xt = x0 + t*p;
            int div1 = xt;
            int div3 = xt+q;

            if (div1 > 255 || div3 > 255)
                break;

            double G1 = ATARI_CLOCK / (div1 + 4.0);
            double G3 = ATARI_CLOCK / (div3 + 4.0);
            double Fsaw = G1-G3;
            double Fcar = G1+G3;

            if (Fcar < 20000.0)
                break;
            if (Fsaw > 20000.0)
                continue;

            printf("%9.2lf, 0x%02x, 0x%02x\n", Fsaw, div1, div3);
        }
    }
}
#endif
