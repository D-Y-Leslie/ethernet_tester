先测一根短线，再测一根长线。目标不是马上精确，而是先看：

```
短线结果小 长线结果大
```

# 短线：1.12m

测量结果：

freq_MHz, dut_amp_dB, ref_amp_dB, rel_phase_deg
   1.000,     -5.07,     -1.19,     -1.25
   1.500,     -3.67,      0.06,     -1.93
   2.000,     -5.15,     -1.32,     -2.94
   2.500,     -4.87,     -1.34,     -3.43
   3.000,     -3.56,      0.10,     -4.13
   3.500,     -4.78,     -1.26,     -5.20
   4.000,     -4.79,     -1.23,     -5.61
   4.500,     -3.57,     -0.07,     -6.72
   5.000,     -4.66,     -1.41,     -7.29
   5.500,     -4.80,     -1.51,     -9.05
   6.000,     -3.39,     -0.15,     -9.89
   6.500,     -4.79,     -1.66,    -10.03
   7.000,     -4.78,     -1.85,    -10.74
   7.500,     -3.53,     -0.61,    -12.73
   8.000,     -4.85,     -2.10,    -12.82
   8.500,     -5.01,     -2.26,    -14.40
   9.000,     -3.79,     -1.16,    -15.66
   9.500,     -5.30,     -2.69,    -17.50
  10.000,     -5.48,     -3.12,    -19.18

Extra delay relative to reference = 5.253 ns
Estimated extra length            = 1.039 m

# 长线：1.1m

测量结果：

freq_MHz, dut_amp_dB, ref_amp_dB, rel_phase_deg
   1.000,     -5.40,      0.16,   -104.76
   1.500,     -3.97,      1.45,   -103.15
   2.000,     -5.47,      0.03,   -101.31
   2.500,     -5.10,     -0.04,   -101.65
   3.000,     -3.59,      1.38,   -100.03
   3.500,     -4.70,      0.04,   -100.81
   4.000,     -4.33,      0.00,   -101.04
   4.500,     -2.68,      1.25,   -102.14
   5.000,     -3.72,     -0.07,   -104.28
   5.500,     -3.74,     -0.24,   -105.79
   6.000,     -2.08,      1.14,   -108.28
   6.500,     -3.44,     -0.40,   -110.09
   7.000,     -3.48,     -0.39,   -112.42
   7.500,     -2.29,      0.71,   -116.08
   8.000,     -3.77,     -0.74,   -117.74
   8.500,     -3.86,     -0.93,   -119.84
   9.000,     -2.84,      0.13,   -122.42
   9.500,     -4.49,     -1.39,   -123.33
  10.000,     -4.76,     -1.70,   -125.61

Extra delay relative to reference = 7.892 ns
Estimated extra length            = 1.562 m

# 同一根 1.12m 线**不动焊点和夹具，连续测 3 次**

## 第一次：

freq_MHz, dut_amp_dB, ref_amp_dB, rel_phase_deg
   0.500,     -9.42,     -5.70,     -5.46
   0.750,     -6.95,     -3.43,     -5.50
   1.000,     -9.32,     -5.67,     -5.81
   1.250,     -9.16,     -5.74,     -6.11
   1.500,     -6.82,     -3.53,     -6.73
   1.750,     -9.03,     -5.66,     -6.94
   2.000,     -8.92,     -5.78,     -7.47
   2.250,     -6.82,     -3.55,     -7.78
   2.500,     -8.95,     -5.71,     -8.35
   2.750,     -9.01,     -5.74,     -8.98
   3.000,     -6.64,     -3.44,     -9.43
   3.250,     -8.83,     -5.79,     -9.40
   3.500,     -8.93,     -5.66,    -10.44
   3.750,     -6.54,     -3.52,    -10.69
   4.000,     -8.93,     -5.78,    -10.57
   4.250,     -8.94,     -5.79,    -11.48
   4.500,     -6.46,     -3.43,    -11.78
   4.750,     -8.92,     -5.83,    -12.53
   5.000,     -8.72,     -5.85,    -12.41
   5.250,     -6.36,     -3.48,    -13.26
   5.500,     -8.68,     -5.84,    -13.63
   5.750,     -8.66,     -5.84,    -14.13
   6.000,     -6.25,     -3.63,    -14.42
   6.250,     -8.54,     -5.88,    -14.96
   6.500,     -8.48,     -5.90,    -15.42
   6.750,     -6.25,     -3.64,    -15.75
   7.000,     -8.44,     -5.94,    -16.51
   7.250,     -8.50,     -5.90,    -16.82
   7.500,     -6.24,     -3.72,    -17.49
   7.750,     -8.34,     -5.92,    -17.87
   8.000,     -8.45,     -6.00,    -18.63
   8.250,     -6.05,     -3.80,    -19.32
   8.500,     -8.42,     -6.11,    -19.48
   8.750,     -8.32,     -5.98,    -20.81
   9.000,     -5.92,     -3.88,    -20.99
   9.250,     -8.19,     -6.16,    -21.43
   9.500,     -8.15,     -6.14,    -21.95
   9.750,     -5.95,     -3.93,    -22.86
  10.000,     -8.21,     -6.28,    -23.22
  10.250,     -8.31,     -6.38,    -24.28
  10.500,     -6.02,     -4.19,    -24.74
  10.750,     -8.13,     -6.45,    -25.46
  11.000,     -8.18,     -6.56,    -26.71
  11.250,     -5.92,     -4.30,    -27.01
  11.500,     -8.42,     -6.77,    -27.78
  11.750,     -8.27,     -6.57,    -28.50
  12.000,     -6.16,     -4.77,    -29.27
  12.250,     -8.42,     -6.86,    -29.98
  12.500,     -8.37,     -7.03,    -30.57
  12.750,     -6.29,     -4.96,    -31.76
  13.000,     -8.63,     -7.37,    -32.35
  13.250,     -8.85,     -7.42,    -33.63
  13.500,     -6.61,     -5.25,    -34.03
  13.750,     -8.98,     -7.82,    -35.22
  14.000,     -9.04,     -7.89,    -35.76

Extra delay relative to reference = 6.158 ns
Estimated extra length            = 1.218 m

## 第二次：

freq_MHz, dut_amp_dB, ref_amp_dB, rel_phase_deg
   0.500,    -10.18,     -6.08,     -2.73
   0.750,     -6.68,     -2.57,     -3.51
   1.000,    -10.24,     -6.18,     -3.77
   1.250,     -9.93,     -6.06,     -4.50
   1.500,     -6.55,     -2.62,     -4.77
   1.750,     -9.89,     -6.04,     -4.77
   2.000,     -9.94,     -6.07,     -5.28
   2.250,     -6.45,     -2.62,     -5.59
   2.500,     -9.91,     -6.20,     -6.23
   2.750,     -9.90,     -6.09,     -6.61
   3.000,     -6.35,     -2.51,     -6.91
   3.250,     -9.88,     -6.11,     -7.05
   3.500,     -9.83,     -6.10,     -7.84
   3.750,     -6.32,     -2.66,     -8.29
   4.000,     -9.63,     -6.11,     -8.26
   4.250,     -9.45,     -6.19,     -9.36
   4.500,     -6.12,     -2.64,     -9.42
   4.750,     -9.70,     -6.13,     -9.84
   5.000,     -9.69,     -6.02,     -9.84
   5.250,     -6.04,     -2.72,    -10.73
   5.500,     -9.59,     -6.21,    -10.86
   5.750,     -9.49,     -6.14,    -11.17
   6.000,     -5.90,     -2.80,    -11.77
   6.250,     -9.61,     -6.25,    -12.03
   6.500,     -9.35,     -6.19,    -12.95
   6.750,     -5.85,     -2.81,    -12.96
   7.000,     -9.30,     -6.26,    -13.57
   7.250,     -9.30,     -6.25,    -13.74
   7.500,     -5.80,     -2.85,    -14.49
   7.750,     -9.19,     -6.38,    -14.97
   8.000,     -9.14,     -6.40,    -15.56
   8.250,     -5.65,     -2.94,    -15.75
   8.500,     -9.19,     -6.36,    -16.27
   8.750,     -9.15,     -6.46,    -17.20
   9.000,     -5.63,     -2.93,    -17.48
   9.250,     -9.07,     -6.44,    -18.35
   9.500,     -9.05,     -6.52,    -18.89
   9.750,     -5.62,     -3.13,    -19.34
  10.000,     -9.13,     -6.78,    -20.30
  10.250,     -9.09,     -6.82,    -20.75
  10.500,     -5.66,     -3.27,    -21.32
  10.750,     -9.20,     -6.83,    -22.63
  11.000,     -9.24,     -6.92,    -23.02
  11.250,     -5.71,     -3.47,    -23.32
  11.500,     -9.09,     -7.10,    -24.20
  11.750,     -9.23,     -7.02,    -24.85
  12.000,     -5.78,     -3.71,    -25.82
  12.250,     -9.26,     -7.31,    -26.45
  12.500,     -9.39,     -7.36,    -26.94
  12.750,     -5.98,     -4.12,    -27.83
  13.000,     -9.54,     -7.75,    -28.66
  13.250,     -9.61,     -7.75,    -29.59
  13.500,     -6.20,     -4.44,    -29.98
  13.750,     -9.82,     -8.11,    -30.50
  14.000,     -9.94,     -8.22,    -31.38

Extra delay relative to reference = 5.724 ns
Estimated extra length            = 1.133 m

## 第三次：

freq_MHz, dut_amp_dB, ref_amp_dB, rel_phase_deg
   0.500,     -6.63,     -1.85,     -3.75
   0.750,     -5.84,     -1.31,     -4.06
   1.000,     -6.50,     -1.88,     -4.25
   1.250,     -6.46,     -1.91,     -4.61
   1.500,     -5.88,     -1.36,     -5.19
   1.750,     -6.42,     -1.85,     -5.85
   2.000,     -6.42,     -1.89,     -6.34
   2.250,     -5.83,     -1.42,     -6.55
   2.500,     -6.46,     -1.95,     -6.62
   2.750,     -6.41,     -1.91,     -7.21
   3.000,     -5.76,     -1.33,     -7.49
   3.250,     -6.32,     -1.90,     -7.57
   3.500,     -6.29,     -2.03,     -8.18
   3.750,     -5.66,     -1.27,     -8.71
   4.000,     -6.33,     -1.94,     -9.02
   4.250,     -6.24,     -1.98,     -9.46
   4.500,     -5.68,     -1.34,     -9.97
   4.750,     -6.17,     -1.97,    -10.09
   5.000,     -6.11,     -1.94,    -10.67
   5.250,     -5.55,     -1.44,    -11.06
   5.500,     -6.04,     -1.95,    -11.73
   5.750,     -6.01,     -1.94,    -12.17
   6.000,     -5.43,     -1.36,    -12.31
   6.250,     -5.97,     -2.01,    -12.78
   6.500,     -6.04,     -2.06,    -13.55
   6.750,     -5.40,     -1.42,    -13.82
   7.000,     -5.97,     -2.08,    -14.32
   7.250,     -5.98,     -2.10,    -14.51
   7.500,     -5.34,     -1.50,    -15.05
   7.750,     -5.75,     -2.13,    -15.61
   8.000,     -5.79,     -2.11,    -16.14
   8.250,     -5.29,     -1.62,    -16.75
   8.500,     -5.72,     -2.20,    -16.94
   8.750,     -5.79,     -2.21,    -17.93
   9.000,     -5.18,     -1.74,    -18.32
   9.250,     -5.70,     -2.25,    -18.66
   9.500,     -5.65,     -2.27,    -19.49
   9.750,     -5.07,     -1.72,    -19.87
  10.000,     -5.66,     -2.47,    -20.55
  10.250,     -5.71,     -2.44,    -21.30
  10.500,     -5.21,     -1.92,    -22.11
  10.750,     -5.65,     -2.57,    -22.54
  11.000,     -5.80,     -2.74,    -23.14
  11.250,     -5.22,     -2.12,    -23.96
  11.500,     -5.80,     -2.70,    -24.68
  11.750,     -5.81,     -2.85,    -25.64
  12.000,     -5.41,     -2.38,    -26.03
  12.250,     -5.90,     -3.04,    -26.90
  12.500,     -5.94,     -3.19,    -27.42
  12.750,     -5.53,     -2.72,    -28.58
  13.000,     -6.17,     -3.39,    -29.13
  13.250,     -6.11,     -3.41,    -29.49
  13.500,     -5.76,     -3.04,    -30.63
  13.750,     -6.49,     -3.70,    -30.98
  14.000,     -6.67,     -3.99,    -32.09

Extra delay relative to reference = 5.686 ns
Estimated extra length            = 1.125 m

# 同一根 1.12m 线**不动焊点和夹具，连续测 3 次

## 第一次

freq_MHz, dut_amp_dB, ref_amp_dB, rel_phase_deg
   0.500,     -7.76,     -3.36,   -111.43
   0.750,     -7.09,     -2.52,   -111.08
   1.000,     -7.96,     -3.45,   -110.83
   1.250,     -8.25,     -3.40,   -110.49
   1.500,     -7.43,     -2.52,   -109.80
   1.750,     -8.23,     -3.33,   -108.52
   2.000,     -8.38,     -3.41,   -108.20
   2.250,     -7.55,     -2.64,   -107.21
   2.500,     -8.41,     -3.33,   -107.15
   2.750,     -8.22,     -3.38,   -105.55
   3.000,     -7.31,     -2.60,   -104.92
   3.250,     -7.90,     -3.50,   -104.05
   3.500,     -7.96,     -3.43,   -103.67
   3.750,     -6.90,     -2.63,   -103.18
   4.000,     -7.58,     -3.43,   -103.27
   4.250,     -7.43,     -3.34,   -103.67
   4.500,     -6.45,     -2.56,   -103.66
   4.750,     -7.11,     -3.48,   -103.86
   5.000,     -6.85,     -3.48,   -104.52
   5.250,     -5.86,     -2.63,   -105.58
   5.500,     -6.65,     -3.56,   -106.35
   5.750,     -6.50,     -3.43,   -107.24
   6.000,     -5.50,     -2.69,   -108.24
   6.250,     -6.25,     -3.49,   -109.04
   6.500,     -6.13,     -3.54,   -110.18
   6.750,     -5.23,     -2.72,   -111.11
   7.000,     -6.10,     -3.54,   -112.69
   7.250,     -6.01,     -3.65,   -113.89
   7.500,     -5.06,     -2.72,   -114.79
   7.750,     -5.87,     -3.62,   -115.81
   8.000,     -5.98,     -3.61,   -117.39
   8.250,     -5.20,     -2.88,   -118.45
   8.500,     -5.98,     -3.73,   -119.31
   8.750,     -6.08,     -3.72,   -121.15
   9.000,     -5.21,     -3.05,   -121.33
   9.250,     -6.13,     -3.84,   -122.98
   9.500,     -6.15,     -3.85,   -124.03
   9.750,     -5.46,     -3.08,   -124.41
  10.000,     -6.33,     -3.94,   -125.52
  10.250,     -6.33,     -4.04,   -126.55
  10.500,     -5.53,     -3.16,   -127.54
  10.750,     -6.58,     -4.11,   -128.79
  11.000,     -6.56,     -4.17,   -129.28
  11.250,     -5.93,     -3.44,   -130.35
  11.500,     -6.72,     -4.24,   -131.49
  11.750,     -6.90,     -4.48,   -132.71
  12.000,     -6.14,     -3.69,   -133.00
  12.250,     -7.15,     -4.59,   -133.40
  12.500,     -7.30,     -4.73,   -134.28
  12.750,     -6.65,     -4.06,   -135.85
  13.000,     -7.65,     -5.10,   -136.35
  13.250,     -7.70,     -5.05,   -137.17
  13.500,     -7.06,     -4.48,   -137.71
  13.750,     -8.18,     -5.50,   -138.80
  14.000,     -8.34,     -5.53,   -139.26

Extra delay relative to reference = 7.478 ns
Estimated extra length            = 1.480 m

## 第二次

freq_MHz, dut_amp_dB, ref_amp_dB, rel_phase_deg
   0.500,     -8.45,     -3.83,   -110.83
   0.750,     -7.25,     -2.54,   -110.89
   1.000,     -8.46,     -3.82,   -110.76
   1.250,     -8.69,     -3.92,   -110.18
   1.500,     -7.58,     -2.67,   -109.35
   1.750,     -8.80,     -3.75,   -108.45
   2.000,     -8.75,     -3.81,   -107.95
   2.250,     -7.76,     -2.64,   -106.63
   2.500,     -8.88,     -3.76,   -105.68
   2.750,     -8.74,     -3.82,   -104.81
   3.000,     -7.50,     -2.54,   -104.16
   3.250,     -8.67,     -3.82,   -103.20
   3.500,     -8.55,     -3.88,   -102.82
   3.750,     -7.18,     -2.64,   -102.53
   4.000,     -8.22,     -3.88,   -102.28
   4.250,     -7.92,     -3.88,   -102.51
   4.500,     -6.56,     -2.63,   -102.62
   4.750,     -7.64,     -3.96,   -102.90
   5.000,     -7.54,     -3.89,   -103.68
   5.250,     -6.16,     -2.68,   -104.41
   5.500,     -7.27,     -3.89,   -105.27
   5.750,     -7.05,     -3.84,   -106.09
   6.000,     -5.75,     -2.73,   -107.10
   6.250,     -7.10,     -3.86,   -108.16
   6.500,     -6.83,     -3.99,   -109.46
   6.750,     -5.66,     -2.71,   -110.58
   7.000,     -6.74,     -3.88,   -111.91
   7.250,     -6.67,     -3.92,   -113.16
   7.500,     -5.43,     -2.80,   -114.19
   7.750,     -6.64,     -4.06,   -114.94
   8.000,     -6.65,     -4.02,   -116.69
   8.250,     -5.50,     -2.91,   -118.10
   8.500,     -6.70,     -4.06,   -119.09
   8.750,     -6.86,     -4.14,   -120.36
   9.000,     -5.51,     -3.09,   -120.82
   9.250,     -6.95,     -4.24,   -122.12
   9.500,     -6.85,     -4.24,   -123.25
   9.750,     -5.76,     -3.14,   -124.20
  10.000,     -7.10,     -4.43,   -125.44
  10.250,     -7.05,     -4.42,   -126.09
  10.500,     -5.97,     -3.23,   -127.27
  10.750,     -7.20,     -4.51,   -128.51
  11.000,     -7.35,     -4.64,   -129.02
  11.250,     -6.22,     -3.53,   -130.07
  11.500,     -7.52,     -4.74,   -131.51
  11.750,     -7.72,     -4.77,   -132.29
  12.000,     -6.56,     -3.81,   -132.79
  12.250,     -7.87,     -4.95,   -133.51
  12.500,     -8.05,     -5.16,   -134.37
  12.750,     -7.12,     -4.08,   -135.26
  13.000,     -8.23,     -5.43,   -135.87
  13.250,     -8.38,     -5.50,   -137.17
  13.500,     -7.53,     -4.50,   -137.56
  13.750,     -9.04,     -5.69,   -138.60
  14.000,     -9.03,     -5.90,   -139.01

Extra delay relative to reference = 7.591 ns
Estimated extra length            = 1.502 m

## 第三次

freq_MHz, dut_amp_dB, ref_amp_dB, rel_phase_deg
   0.500,     -9.08,     -4.87,   -110.83
   0.750,     -7.05,     -2.54,   -110.71
   1.000,     -9.29,     -4.82,   -110.08
   1.250,     -9.39,     -4.76,   -109.49
   1.500,     -7.24,     -2.55,   -108.98
   1.750,     -9.72,     -4.77,   -108.27
   2.000,     -9.56,     -4.77,   -106.96
   2.250,     -7.32,     -2.57,   -106.36
   2.500,     -9.66,     -4.88,   -105.33
   2.750,     -9.43,     -4.75,   -104.48
   3.000,     -7.21,     -2.61,   -103.62
   3.250,     -9.24,     -4.86,   -103.27
   3.500,     -9.02,     -4.86,   -103.13
   3.750,     -6.74,     -2.60,   -102.35
   4.000,     -8.70,     -4.74,   -102.77
   4.250,     -8.52,     -4.81,   -102.69
   4.500,     -6.24,     -2.69,   -102.96
   4.750,     -8.39,     -4.81,   -103.60
   5.000,     -8.19,     -4.79,   -103.92
   5.250,     -5.76,     -2.71,   -104.79
   5.500,     -7.94,     -4.81,   -106.27
   5.750,     -7.67,     -4.94,   -107.09
   6.000,     -5.42,     -2.64,   -107.99
   6.250,     -7.41,     -4.90,   -109.04
   6.500,     -7.46,     -4.90,   -110.23
   6.750,     -5.25,     -2.77,   -110.90
   7.000,     -7.42,     -4.94,   -112.48
   7.250,     -7.39,     -4.95,   -113.49
   7.500,     -5.26,     -2.69,   -114.65
   7.750,     -7.42,     -4.96,   -115.88
   8.000,     -7.31,     -5.07,   -117.18
   8.250,     -5.18,     -2.95,   -118.37
   8.500,     -7.50,     -5.07,   -119.13
   8.750,     -7.41,     -5.13,   -120.72
   9.000,     -5.26,     -2.88,   -121.37
   9.250,     -7.49,     -5.13,   -122.63
   9.500,     -7.51,     -5.12,   -123.47
   9.750,     -5.40,     -3.19,   -124.75
  10.000,     -7.68,     -5.28,   -125.78
  10.250,     -7.66,     -5.27,   -126.49
  10.500,     -5.57,     -3.16,   -127.67
  10.750,     -8.03,     -5.59,   -128.21
  11.000,     -7.93,     -5.49,   -129.39
  11.250,     -5.85,     -3.35,   -130.36
  11.500,     -8.19,     -5.64,   -131.18
  11.750,     -8.37,     -5.76,   -132.16
  12.000,     -6.22,     -3.53,   -132.96
  12.250,     -8.47,     -5.88,   -133.85
  12.500,     -8.73,     -6.01,   -134.86
  12.750,     -6.61,     -3.93,   -135.51
  13.000,     -8.96,     -6.31,   -135.83
  13.250,     -9.14,     -6.28,   -137.17
  13.500,     -7.14,     -4.25,   -137.86
  13.750,     -9.44,     -6.64,   -138.62
  14.000,     -9.79,     -6.75,   -139.40

Extra delay relative to reference = 7.691 ns
Estimated extra length            = 1.522 m

# **实验 A：双回环通道互换**

## A1: TX1A -> RX1A, TX2A -> RX2A

Normal 2T2R loopback

- TX1A -> RX1A
- TX2A -> RX2A
- Use the two short SMA paths without RJ45 cable.


Ready for test: Normal 2T2R loopback
TX output is OFF. change hardware now, then keep your hands off the wiring.
Press Enter to START this test...
TX multitone ON
normal_loopback      run 1: tau=    0.277 ns, signed_len=   0.055 m, abs_len=   0.055 m, valid=55, dut_min=  -4.76 dB, resid=  0.12 deg, OK
normal_loopback      run 2: tau=    0.249 ns, signed_len=   0.049 m, abs_len=   0.049 m, valid=55, dut_min=  -4.75 dB, resid=  0.11 deg, OK
normal_loopback      run 3: tau=    0.236 ns, signed_len=   0.047 m, abs_len=   0.047 m, valid=55, dut_min=  -4.76 dB, resid=  0.13 deg, OK

Stage summary:
  stage                = normal_loopback
  runs                 = 3
  mean tau             = 0.254 ns
  mean signed length   = 0.050 m
  signed length span   = 0.008 m
  min valid tones      = 55
  max residual         = 0.13 deg
  verdict              = OK
TX output is OFF. You may change hardware before the next test prompt.

RX channel swapped loopback

- TX1A -> RX2A
- TX2A -> RX1A
- This exposes TX/RX channel bias and sign changes.

Ready for test: RX channel swapped loopback
TX output is OFF. change hardware now, then keep your hands off the wiring.

## A2: TX1A -> RX2A, TX2A -> RX1A

- TX1A -> RX2A
- TX2A -> RX1A
- This exposes TX/RX channel bias and sign changes.


Ready for test: RX channel swapped loopback
TX output is OFF. change hardware now, then keep your hands off the wiring.
Press Enter to START this test...
TX multitone ON
rx_swapped_loopback  run 1: tau=  313.666 ns, signed_len=  62.063 m, abs_len=  62.063 m, valid= 0, dut_min= -49.70 dB, ref_min= -49.04 dB, resid=154.67 deg, LOW_SIGNAL
rx_swapped_loopback  run 2: tau= -115.613 ns, signed_len= -22.876 m, abs_len=  22.876 m, valid= 0, dut_min= -48.55 dB, ref_min= -51.60 dB, resid=139.07 deg, LOW_SIGNAL
rx_swapped_loopback  run 3: tau=  207.660 ns, signed_len=  41.088 m, abs_len=  41.088 m, valid= 0, dut_min= -50.16 dB, ref_min= -49.42 dB, resid=122.56 deg, LOW_SIGNAL

Stage summary:
  stage                = rx_swapped_loopback
  runs                 = 3
  mean tau             = 135.238 ns
  mean signed length   = 26.759 m
  signed length span   = 84.938 m
  min valid tones      = 0
  max residual         = 154.67 deg
  verdict              = LOW_SIGNAL
TX output is OFF. You may change hardware before the next test prompt.

SMA A on DUT, SMA B on REF

- SMA A: TX1A -> RX1A
- SMA B: TX2A -> RX2A
- Mark the physical SMA cables before this run.



## B1:

SMA1: TX1A -> RX1A
SMA2: TX2A -> RX2A

- SMA A: TX1A -> RX1A
- SMA B: TX2A -> RX2A
- Mark the physical SMA cables before this run.


Ready for test: SMA A on DUT, SMA B on REF
TX output is OFF. change hardware now, then keep your hands off the wiring.
Press Enter to START this test...
TX multitone ON
sma_a_dut_b_ref      run 1: tau=    0.285 ns, signed_len=   0.056 m, abs_len=   0.056 m, valid=55, dut_min=  -4.78 dB, ref_min=  -4.72 dB, resid=  0.16 deg, OK
sma_a_dut_b_ref      run 2: tau=    0.306 ns, signed_len=   0.061 m, abs_len=   0.061 m, valid=55, dut_min=  -4.75 dB, ref_min=  -4.75 dB, resid=  0.15 deg, OK
sma_a_dut_b_ref      run 3: tau=    0.272 ns, signed_len=   0.054 m, abs_len=   0.054 m, valid=55, dut_min=  -4.78 dB, ref_min=  -4.74 dB, resid=  0.14 deg, OK

Stage summary:
  stage                = sma_a_dut_b_ref
  runs                 = 3
  mean tau             = 0.288 ns
  mean signed length   = 0.057 m
  signed length span   = 0.007 m
  min valid tones      = 55
  max residual         = 0.16 deg
  verdict              = OK
TX output is OFF. You may change hardware before the next test prompt.

SMA B on DUT, SMA A on REF

- SMA B: TX1A -> RX1A
- SMA A: TX2A -> RX2A
- The signed result should roughly flip from the previous stage.

## B2:

SMA2: TX1A -> RX1A
SMA1: TX2A -> RX2A

- SMA B: TX1A -> RX1A
- SMA A: TX2A -> RX2A
- The signed result should roughly flip from the previous stage.


Ready for test: SMA B on DUT, SMA A on REF
TX output is OFF. change hardware now, then keep your hands off the wiring.
Press Enter to START this test...
TX multitone ON
sma_b_dut_a_ref      run 1: tau=    0.265 ns, signed_len=   0.052 m, abs_len=   0.052 m, valid=55, dut_min=  -4.93 dB, ref_min=  -4.63 dB, resid=  0.16 deg, OK
sma_b_dut_a_ref      run 2: tau=    0.275 ns, signed_len=   0.054 m, abs_len=   0.054 m, valid=55, dut_min=  -4.89 dB, ref_min=  -4.60 dB, resid=  0.15 deg, OK
sma_b_dut_a_ref      run 3: tau=    0.262 ns, signed_len=   0.052 m, abs_len=   0.052 m, valid=55, dut_min=  -4.89 dB, ref_min=  -4.62 dB, resid=  0.15 deg, OK

Stage summary:
  stage                = sma_b_dut_a_ref
  runs                 = 3
  mean tau             = 0.267 ns
  mean signed length   = 0.053 m
  signed length span   = 0.002 m
  min valid tones      = 55
  max residual         = 0.16 deg
  verdict              = OK
TX output is OFF. You may change hardware before the next test prompt.

Stopping TX...

## 先跑一次正常回环

Ready for test: DUT cable forward
TX output is OFF. change hardware now, then keep your hands off the wiring.
Press Enter to START this test...
TX multitone ON
dut_forward          run 1: tau=    0.311 ns, signed_len=   0.062 m, corr_signed=   0.007 m, abs_len=   0.062 m, valid=55, dut_min=  -4.62 dB, ref_min=  -4.57 dB, resid=  0.14 deg, OK
dut_forward          run 2: tau=    0.331 ns, signed_len=   0.065 m, corr_signed=   0.010 m, abs_len=   0.065 m, valid=55, dut_min=  -4.61 dB, ref_min=  -4.58 dB, resid=  0.15 deg, OK
dut_forward          run 3: tau=    0.323 ns, signed_len=   0.064 m, corr_signed=   0.009 m, abs_len=   0.064 m, valid=55, dut_min=  -4.62 dB, ref_min=  -4.57 dB, resid=  0.15 deg, OK
dut_forward          run 4: tau=    0.311 ns, signed_len=   0.062 m, corr_signed=   0.007 m, abs_len=   0.062 m, valid=55, dut_min=  -4.64 dB, ref_min=  -4.58 dB, resid=  0.17 deg, OK
dut_forward          run 5: tau=    0.331 ns, signed_len=   0.066 m, corr_signed=   0.011 m, abs_len=   0.066 m, valid=55, dut_min=  -4.61 dB, ref_min=  -4.56 dB, resid=  0.16 deg, OK

Stage summary:
  stage                = dut_forward
  runs                 = 5
  mean tau             = 0.321 ns
  mean signed length   = 0.064 m
  corrected signed len = 0.009 m
  signed length span   = 0.004 m
  corrected span       = 0.004 m
  min valid tones      = 55
  max residual         = 0.17 deg
  verdict              = OK
TX output is OFF. You may change hardware before the next test prompt.

## 测 1.12 m

### 正

DUT cable reversed

- Reverse the same RJ45 cable end-to-end on the DUT path.
- Keep TX1A/RX1A and TX2A/RX2A channel roles unchanged.
- This checks connector/fixture direction sensitivity.


Ready for test: DUT cable reversed
TX output is OFF. change hardware now, then keep your hands off the wiring.
Press Enter to START this test...                  
TX multitone ON
dut_reversed         run 1: tau=    5.932 ns, signed_len=   1.174 m, corr_signed=   1.119 m, abs_len=   1.174 m, valid=55, dut_min=  -6.47 dB, ref_min=  -4.56 dB, resid=  1.01 deg, OK
dut_reversed         run 2: tau=    5.931 ns, signed_len=   1.174 m, corr_signed=   1.119 m, abs_len=   1.174 m, valid=55, dut_min=  -6.47 dB, ref_min=  -4.56 dB, resid=  1.01 deg, OK
dut_reversed         run 3: tau=    5.915 ns, signed_len=   1.170 m, corr_signed=   1.115 m, abs_len=   1.170 m, valid=55, dut_min=  -6.48 dB, ref_min=  -4.53 dB, resid=  0.97 deg, OK
dut_reversed         run 4: tau=    5.912 ns, signed_len=   1.170 m, corr_signed=   1.115 m, abs_len=   1.170 m, valid=55, dut_min=  -6.47 dB, ref_min=  -4.54 dB, resid=  1.02 deg, OK
dut_reversed         run 5: tau=    5.925 ns, signed_len=   1.172 m, corr_signed=   1.117 m, abs_len=   1.172 m, valid=55, dut_min=  -6.46 dB, ref_min=  -4.57 dB, resid=  0.99 deg, OK

Stage summary:
  stage                = dut_reversed
  runs                 = 5
  mean tau             = 5.923 ns
  mean signed length   = 1.172 m
  corrected signed len = 1.117 m
  signed length span   = 0.004 m
  corrected span       = 0.004 m
  min valid tones      = 55
  max residual         = 1.02 deg
  verdict              = OK
TX output is OFF. You may change hardware before the next test prompt.

(.venv) PS C:\Users\Leslie\Documents\Arduino\Project\ethernet_tester> & "D:\Python\Python File\Professional Experiments\.venv\Scripts\python.exe" -B .\pluto_2x2_channel_swap_test.py --runs 5 --zero-offset-m 0.055 --stages dut_forward dut_reversed
AD9361 2T2R channel/SMA swap experiment

Recommended stage order:
- dut_forward          (DUT-length)
- dut_reversed         (DUT-length)

Keep signed_length_m. A sign flip after swapping cables/channels is useful evidence.
Using zero offset correction: 0.055 m
Press Enter after checking that RF cables are safe to connect...
Connecting to ip:192.168.2.1...
Configuring AD9361 2T2R...

========================================================================
DUT cable forward
========================================================================
- DUT path: TX1A -> SMA/RJ45 fixture -> cable -> SMA/RJ45 fixture -> RX1A
- Reference path: TX2A -> RX2A with the chosen reference SMA cable.
- Keep the reference SMA cable fixed for all DUT cable runs.


Ready for test: DUT cable forward
TX output is OFF. change hardware now, then keep your hands off the wiring.
Press Enter to START this test...
TX multitone ON
dut_forward          run 1: tau=    5.784 ns, signed_len=   1.145 m, corr_signed=   1.090 m, abs_len=   1.145 m, valid=55, dut_min=  -6.10 dB, ref_min=  -3.48 dB, resid=  0.97 deg, OK
dut_forward          run 2: tau=    5.775 ns, signed_len=   1.143 m, corr_signed=   1.088 m, abs_len=   1.143 m, valid=55, dut_min=  -6.09 dB, ref_min=  -3.49 dB, resid=  0.97 deg, OK
dut_forward          run 3: tau=    5.787 ns, signed_len=   1.145 m, corr_signed=   1.090 m, abs_len=   1.145 m, valid=55, dut_min=  -6.10 dB, ref_min=  -3.49 dB, resid=  0.99 deg, OK
dut_forward          run 4: tau=    5.787 ns, signed_len=   1.145 m, corr_signed=   1.090 m, abs_len=   1.145 m, valid=55, dut_min=  -6.12 dB, ref_min=  -3.49 dB, resid=  0.96 deg, OK
dut_forward          run 5: tau=    5.775 ns, signed_len=   1.143 m, corr_signed=   1.088 m, abs_len=   1.143 m, valid=55, dut_min=  -6.09 dB, ref_min=  -3.47 dB, resid=  0.96 deg, OK

Stage summary:
  stage                = dut_forward
  runs                 = 5
  mean tau             = 5.782 ns
  mean signed length   = 1.144 m
  corrected signed len = 1.089 m
  signed length span   = 0.002 m
  corrected span       = 0.002 m
  min valid tones      = 55
  max residual         = 0.99 deg
  verdict              = OK
TX output is OFF. You may change hardware before the next test prompt.

========================================================================
DUT cable reversed
========================================================================
- Reverse the same RJ45 cable end-to-end on the DUT path.
- Keep TX1A/RX1A and TX2A/RX2A channel roles unchanged.
- This checks connector/fixture direction sensitivity.


Ready for test: DUT cable reversed
TX output is OFF. change hardware now, then keep your hands off the wiring.
Press Enter to START this test...
TX multitone ON
dut_reversed         run 1: tau=    5.782 ns, signed_len=   1.144 m, corr_signed=   1.089 m, abs_len=   1.144 m, valid=55, dut_min=  -6.08 dB, ref_min=  -3.50 dB, resid=  0.96 deg, OK
dut_reversed         run 2: tau=    5.762 ns, signed_len=   1.140 m, corr_signed=   1.085 m, abs_len=   1.140 m, valid=55, dut_min=  -6.08 dB, ref_min=  -3.47 dB, resid=  0.97 deg, OK
dut_reversed         run 3: tau=    5.759 ns, signed_len=   1.139 m, corr_signed=   1.084 m, abs_len=   1.139 m, valid=55, dut_min=  -6.10 dB, ref_min=  -3.50 dB, resid=  0.96 deg, OK
dut_reversed         run 4: tau=    5.785 ns, signed_len=   1.145 m, corr_signed=   1.090 m, abs_len=   1.145 m, valid=55, dut_min=  -6.07 dB, ref_min=  -3.50 dB, resid=  0.96 deg, OK
dut_reversed         run 5: tau=    5.761 ns, signed_len=   1.140 m, corr_signed=   1.085 m, abs_len=   1.140 m, valid=55, dut_min=  -6.10 dB, ref_min=  -3.49 dB, resid=  0.96 deg, OK

Stage summary:
  stage                = dut_reversed
  runs                 = 5
  mean tau             = 5.770 ns
  mean signed length   = 1.142 m
  corrected signed len = 1.087 m
  signed length span   = 0.005 m
  corrected span       = 0.005 m
  min valid tones      = 55
  max residual         = 0.97 deg
  verdict              = OK
TX output is OFF. You may change hardware before the next test prompt.

========================================================================
DUT cable forward
========================================================================
- DUT path: TX1A -> SMA/RJ45 fixture -> cable -> SMA/RJ45 fixture -> RX1A
- Reference path: TX2A -> RX2A with the chosen reference SMA cable.
- Keep the reference SMA cable fixed for all DUT cable runs.


Ready for test: DUT cable forward
TX output is OFF. change hardware now, then keep your hands off the wiring.
Press Enter to START this test...
TX multitone ON
dut_forward          run 1: tau=    6.222 ns, signed_len=   1.231 m, corr_signed=   1.176 m, abs_len=   1.231 m, valid=55, dut_min=  -6.94 dB, ref_min=  -5.60 dB, resid=  1.04 deg, OK
dut_forward          run 2: tau=    6.222 ns, signed_len=   1.231 m, corr_signed=   1.176 m, abs_len=   1.231 m, valid=55, dut_min=  -6.96 dB, ref_min=  -5.58 dB, resid=  1.08 deg, OK
dut_forward          run 3: tau=    6.232 ns, signed_len=   1.233 m, corr_signed=   1.178 m, abs_len=   1.233 m, valid=55, dut_min=  -6.98 dB, ref_min=  -5.60 dB, resid=  1.09 deg, OK
dut_forward          run 4: tau=    6.240 ns, signed_len=   1.235 m, corr_signed=   1.180 m, abs_len=   1.235 m, valid=55, dut_min=  -6.96 dB, ref_min=  -5.60 dB, resid=  1.06 deg, OK
dut_forward          run 5: tau=    6.238 ns, signed_len=   1.234 m, corr_signed=   1.179 m, abs_len=   1.234 m, valid=55, dut_min=  -6.98 dB, ref_min=  -5.59 dB, resid=  1.09 deg, OK

Stage summary:
  stage                = dut_forward
  runs                 = 5
  mean tau             = 6.231 ns
  mean signed length   = 1.233 m
  corrected signed len = 1.178 m
  signed length span   = 0.004 m
  corrected span       = 0.004 m
  min valid tones      = 55
  max residual         = 1.09 deg
  verdict              = OK
TX output is OFF. You may change hardware before the next test prompt.

========================================================================
DUT cable reversed
========================================================================
- Reverse the same RJ45 cable end-to-end on the DUT path.
- Keep TX1A/RX1A and TX2A/RX2A channel roles unchanged.
- This checks connector/fixture direction sensitivity.


Ready for test: DUT cable reversed
TX output is OFF. change hardware now, then keep your hands off the wiring.
Press Enter to START this test...
TX multitone ON
dut_reversed         run 1: tau=    6.243 ns, signed_len=   1.235 m, corr_signed=   1.180 m, abs_len=   1.235 m, valid=55, dut_min=  -6.98 dB, ref_min=  -5.59 dB, resid=  1.05 deg, OK
dut_reversed         run 2: tau=    6.204 ns, signed_len=   1.227 m, corr_signed=   1.172 m, abs_len=   1.227 m, valid=55, dut_min=  -6.96 dB, ref_min=  -5.59 dB, resid=  1.06 deg, OK
dut_reversed         run 3: tau=    6.291 ns, signed_len=   1.245 m, corr_signed=   1.190 m, abs_len=   1.245 m, valid=55, dut_min=  -7.06 dB, ref_min=  -5.58 dB, resid=  1.00 deg, OK
dut_reversed         run 4: tau=    6.451 ns, signed_len=   1.276 m, corr_signed=   1.221 m, abs_len=   1.276 m, valid=55, dut_min=  -6.88 dB, ref_min=  -5.62 dB, resid=  0.94 deg, OK
dut_reversed         run 5: tau=    6.434 ns, signed_len=   1.273 m, corr_signed=   1.218 m, abs_len=   1.273 m, valid=55, dut_min=  -6.90 dB, ref_min=  -5.56 dB, resid=  0.96 deg, OK

Stage summary:
  stage                = dut_reversed
  runs                 = 5
  mean tau             = 6.324 ns
  mean signed length   = 1.251 m
  corrected signed len = 1.196 m
  signed length span   = 0.049 m
  corrected span       = 0.049 m
  min valid tones      = 55
  max residual         = 1.06 deg
  verdict              = OK
TX output is OFF. You may change hardware before the next test prompt.

### 反

DUT cable forward
========================================================================
- DUT path: TX1A -> SMA/RJ45 fixture -> cable -> SMA/RJ45 fixture -> RX1A
- Reference path: TX2A -> RX2A with the chosen reference SMA cable.
- Keep the reference SMA cable fixed for all DUT cable runs.


Ready for test: DUT cable forward
TX output is OFF. change hardware now, then keep your hands off the wiring.
Press Enter to START this test...
TX multitone ON
dut_forward          run 1: tau=    4.752 ns, signed_len=   0.940 m, corr_signed=   0.885 m, abs_len=   0.940 m, valid=55, dut_min=  -8.07 dB, ref_min=  -3.51 dB, resid=  0.87 deg, OK
dut_forward          run 2: tau=    4.366 ns, signed_len=   0.864 m, corr_signed=   0.809 m, abs_len=   0.864 m, valid=55, dut_min=  -8.19 dB, ref_min=  -3.49 dB, resid=  0.92 deg, OK
dut_forward          run 3: tau=    4.360 ns, signed_len=   0.863 m, corr_signed=   0.808 m, abs_len=   0.863 m, valid=55, dut_min=  -8.27 dB, ref_min=  -3.49 dB, resid=  0.88 deg, OK
dut_forward          run 4: tau=    4.346 ns, signed_len=   0.860 m, corr_signed=   0.805 m, abs_len=   0.860 m, valid=55, dut_min=  -8.20 dB, ref_min=  -3.52 dB, resid=  0.89 deg, OK
dut_forward          run 5: tau=    4.334 ns, signed_len=   0.858 m, corr_signed=   0.803 m, abs_len=   0.858 m, valid=55, dut_min=  -8.25 dB, ref_min=  -3.51 dB, resid=  0.92 deg, OK

Stage summary:
  stage                = dut_forward
  runs                 = 5
  mean tau             = 4.432 ns
  mean signed length   = 0.877 m
  corrected signed len = 0.822 m
  signed length span   = 0.083 m
  corrected span       = 0.083 m
  min valid tones      = 55
  max residual         = 0.92 deg
  verdict              = OK
TX output is OFF. You may change hardware before the next test prompt.

========================================================================
DUT cable reversed
========================================================================
- Reverse the same RJ45 cable end-to-end on the DUT path.
- Keep TX1A/RX1A and TX2A/RX2A channel roles unchanged.
- This checks connector/fixture direction sensitivity.

DUT cable reversed
========================================================================
- Reverse the same RJ45 cable end-to-end on the DUT path.
- Keep TX1A/RX1A and TX2A/RX2A channel roles unchanged.
- This checks connector/fixture direction sensitivity.


Ready for test: DUT cable reversed
TX output is OFF. change hardware now, then keep your hands off the wiring.
Press Enter to START this test...
TX multitone ON
dut_reversed         run 1: tau=    4.955 ns, signed_len=   0.980 m, corr_signed=   0.925 m, abs_len=   0.980 m, valid=55, dut_min=  -7.63 dB, ref_min=  -3.48 dB, resid=  0.91 deg, OK
dut_reversed         run 2: tau=    5.844 ns, signed_len=   1.156 m, corr_signed=   1.101 m, abs_len=   1.156 m, valid=55, dut_min=  -6.11 dB, ref_min=  -3.50 dB, resid=  1.07 deg, OK
dut_reversed         run 3: tau=    5.833 ns, signed_len=   1.154 m, corr_signed=   1.099 m, abs_len=   1.154 m, valid=55, dut_min=  -6.27 dB, ref_min=  -3.48 dB, resid=  1.03 deg, OK
dut_reversed         run 4: tau=    5.777 ns, signed_len=   1.143 m, corr_signed=   1.088 m, abs_len=   1.143 m, valid=55, dut_min=  -6.49 dB, ref_min=  -3.52 dB, resid=  1.01 deg, OK
dut_reversed         run 5: tau=    5.640 ns, signed_len=   1.116 m, corr_signed=   1.061 m, abs_len=   1.116 m, valid=55, dut_min=  -6.89 dB, ref_min=  -3.49 dB, resid=  0.95 deg, OK

Stage summary:
  stage                = dut_reversed
  runs                 = 5
  mean tau             = 5.610 ns
  mean signed length   = 1.110 m
  corrected signed len = 1.055 m
  signed length span   = 0.176 m
  corrected span       = 0.176 m
  min valid tones      = 55
  max residual         = 1.07 deg
  verdict              = OK
TX output is OFF. You may change hardware before the next test prompt.

## 测1.1m

### 正

- DUT path: TX1A -> SMA/RJ45 fixture -> cable -> SMA/RJ45 fixture -> RX1A
- Reference path: TX2A -> RX2A with the chosen reference SMA cable.
- Keep the reference SMA cable fixed for all DUT cable runs.


Ready for test: DUT cable forward
TX output is OFF. change hardware now, then keep your hands off the wiring.
Press Enter to START this test...
TX multitone ON
dut_forward          run 1: tau=    7.656 ns, signed_len=   1.515 m, corr_signed=   1.460 m, abs_len=   1.515 m, valid=55, dut_min=  -7.70 dB, ref_min=  -4.76 dB, resid=  5.14 deg, OK
dut_forward          run 2: tau=    7.718 ns, signed_len=   1.527 m, corr_signed=   1.472 m, abs_len=   1.527 m, valid=55, dut_min=  -7.61 dB, ref_min=  -4.75 dB, resid=  5.14 deg, OK
dut_forward          run 3: tau=    7.701 ns, signed_len=   1.524 m, corr_signed=   1.469 m, abs_len=   1.524 m, valid=55, dut_min=  -7.59 dB, ref_min=  -4.77 dB, resid=  5.16 deg, OK
dut_forward          run 4: tau=    7.729 ns, signed_len=   1.529 m, corr_signed=   1.474 m, abs_len=   1.529 m, valid=55, dut_min=  -7.61 dB, ref_min=  -4.75 dB, resid=  5.15 deg, OK
dut_forward          run 5: tau=    7.726 ns, signed_len=   1.529 m, corr_signed=   1.474 m, abs_len=   1.529 m, valid=55, dut_min=  -7.60 dB, ref_min=  -4.76 dB, resid=  5.17 deg, OK

Stage summary:
  stage                = dut_forward
  runs                 = 5
  mean tau             = 7.706 ns
  mean signed length   = 1.525 m
  corrected signed len = 1.470 m
  signed length span   = 0.014 m
  corrected span       = 0.014 m
  min valid tones      = 55
  max residual         = 5.17 deg
  verdict              = OK
TX output is OFF. You may change hardware before the next test prompt.

========================================================================
DUT cable reversed
========================================================================
- Reverse the same RJ45 cable end-to-end on the DUT path.
- Keep TX1A/RX1A and TX2A/RX2A channel roles unchanged.
- This checks connector/fixture direction sensitivity.


Ready for test: DUT cable reversed
TX output is OFF. change hardware now, then keep your hands off the wiring.
Press Enter to START this test...
TX multitone ON
dut_reversed         run 1: tau=    7.672 ns, signed_len=   1.518 m, corr_signed=   1.463 m, abs_len=   1.518 m, valid=55, dut_min=  -7.65 dB, ref_min=  -4.76 dB, resid=  5.16 deg, OK
dut_reversed         run 2: tau=    7.615 ns, signed_len=   1.507 m, corr_signed=   1.452 m, abs_len=   1.507 m, valid=55, dut_min=  -7.65 dB, ref_min=  -4.75 dB, resid=  5.22 deg, OK
dut_reversed         run 3: tau=    7.636 ns, signed_len=   1.511 m, corr_signed=   1.456 m, abs_len=   1.511 m, valid=55, dut_min=  -7.65 dB, ref_min=  -4.75 dB, resid=  5.22 deg, OK
dut_reversed         run 4: tau=    7.632 ns, signed_len=   1.510 m, corr_signed=   1.455 m, abs_len=   1.510 m, valid=55, dut_min=  -7.63 dB, ref_min=  -4.74 dB, resid=  5.24 deg, OK
dut_reversed         run 5: tau=    7.621 ns, signed_len=   1.508 m, corr_signed=   1.453 m, abs_len=   1.508 m, valid=55, dut_min=  -7.65 dB, ref_min=  -4.75 dB, resid=  5.21 deg, OK

Stage summary:
  stage                = dut_reversed
  runs                 = 5
  mean tau             = 7.635 ns
  mean signed length   = 1.511 m
  corrected signed len = 1.456 m
  signed length span   = 0.011 m
  corrected span       = 0.011 m
  min valid tones      = 55
  max residual         = 5.24 deg
  verdict              = OK
TX output is OFF. You may change hardware before the next test prompt.

========================================================================
DUT cable forward
========================================================================
- DUT path: TX1A -> SMA/RJ45 fixture -> cable -> SMA/RJ45 fixture -> RX1A
- Reference path: TX2A -> RX2A with the chosen reference SMA cable.
- Keep the reference SMA cable fixed for all DUT cable runs.


Ready for test: DUT cable forward
TX output is OFF. change hardware now, then keep your hands off the wiring.
Press Enter to START this test...
TX multitone ON
dut_forward          run 1: tau=    7.959 ns, signed_len=   1.575 m, corr_signed=   1.520 m, abs_len=   1.575 m, valid=55, dut_min=  -6.85 dB, ref_min=  -3.44 dB, resid=  4.38 deg, OK
dut_forward          run 2: tau=    7.954 ns, signed_len=   1.574 m, corr_signed=   1.519 m, abs_len=   1.574 m, valid=55, dut_min=  -6.87 dB, ref_min=  -3.47 dB, resid=  4.37 deg, OK
dut_forward          run 3: tau=    7.954 ns, signed_len=   1.574 m, corr_signed=   1.519 m, abs_len=   1.574 m, valid=55, dut_min=  -6.84 dB, ref_min=  -3.44 dB, resid=  4.37 deg, OK
dut_forward          run 4: tau=    7.936 ns, signed_len=   1.570 m, corr_signed=   1.515 m, abs_len=   1.570 m, valid=55, dut_min=  -6.85 dB, ref_min=  -3.45 dB, resid=  4.38 deg, OK
dut_forward          run 5: tau=    7.984 ns, signed_len=   1.580 m, corr_signed=   1.525 m, abs_len=   1.580 m, valid=55, dut_min=  -6.83 dB, ref_min=  -3.46 dB, resid=  4.36 deg, OK

Stage summary:
  stage                = dut_forward
  runs                 = 5
  mean tau             = 7.958 ns
  mean signed length   = 1.574 m
  corrected signed len = 1.519 m
  signed length span   = 0.010 m
  corrected span       = 0.010 m
  min valid tones      = 55
  max residual         = 4.38 deg
  verdict              = OK
TX output is OFF. You may change hardware before the next test prompt.

========================================================================
DUT cable reversed
========================================================================
- Reverse the same RJ45 cable end-to-end on the DUT path.
- Keep TX1A/RX1A and TX2A/RX2A channel roles unchanged.
- This checks connector/fixture direction sensitivity.


Ready for test: DUT cable reversed
TX output is OFF. change hardware now, then keep your hands off the wiring.
Press Enter to START this test...
TX multitone ON
dut_reversed         run 1: tau=    7.952 ns, signed_len=   1.573 m, corr_signed=   1.518 m, abs_len=   1.573 m, valid=55, dut_min=  -6.86 dB, ref_min=  -3.46 dB, resid=  4.44 deg, OK
dut_reversed         run 2: tau=    7.944 ns, signed_len=   1.572 m, corr_signed=   1.517 m, abs_len=   1.572 m, valid=55, dut_min=  -6.83 dB, ref_min=  -3.45 dB, resid=  4.39 deg, OK
dut_reversed         run 3: tau=    7.948 ns, signed_len=   1.573 m, corr_signed=   1.518 m, abs_len=   1.573 m, valid=55, dut_min=  -6.87 dB, ref_min=  -3.45 dB, resid=  4.36 deg, OK
dut_reversed         run 4: tau=    7.940 ns, signed_len=   1.571 m, corr_signed=   1.516 m, abs_len=   1.571 m, valid=55, dut_min=  -6.85 dB, ref_min=  -3.46 dB, resid=  4.41 deg, OK
dut_reversed         run 5: tau=    7.942 ns, signed_len=   1.571 m, corr_signed=   1.516 m, abs_len=   1.571 m, valid=55, dut_min=  -6.85 dB, ref_min=  -3.46 dB, resid=  4.42 deg, OK

Stage summary:
  stage                = dut_reversed
  runs                 = 5
  mean tau             = 7.945 ns
  mean signed length   = 1.572 m
  corrected signed len = 1.517 m
  signed length span   = 0.002 m
  corrected span       = 0.002 m
  min valid tones      = 55
  max residual         = 4.44 deg
  verdict              = OK
TX output is OFF. You may change hardware before the next test prompt.

### 反

========================================================================
DUT cable forward
========================================================================
- DUT path: TX1A -> SMA/RJ45 fixture -> cable -> SMA/RJ45 fixture -> RX1A
- Reference path: TX2A -> RX2A with the chosen reference SMA cable.
- Keep the reference SMA cable fixed for all DUT cable runs.


Ready for test: DUT cable forward
TX output is OFF. change hardware now, then keep your hands off the wiring.
Press Enter to START this test...
TX multitone ON
dut_forward          run 1: tau=    8.534 ns, signed_len=   1.689 m, corr_signed=   1.634 m, abs_len=   1.689 m, valid=55, dut_min=  -8.07 dB, ref_min=  -4.58 dB, resid=  4.76 deg, OK
dut_forward          run 2: tau=    8.576 ns, signed_len=   1.697 m, corr_signed=   1.642 m, abs_len=   1.697 m, valid=55, dut_min=  -8.28 dB, ref_min=  -4.57 dB, resid=  4.75 deg, OK
dut_forward          run 3: tau=    8.255 ns, signed_len=   1.633 m, corr_signed=   1.578 m, abs_len=   1.633 m, valid=55, dut_min=  -8.22 dB, ref_min=  -4.56 dB, resid=  4.56 deg, OK
dut_forward          run 4: tau=    8.473 ns, signed_len=   1.676 m, corr_signed=   1.621 m, abs_len=   1.676 m, valid=55, dut_min=  -7.96 dB, ref_min=  -4.56 dB, resid=  4.64 deg, OK
dut_forward          run 5: tau=    8.415 ns, signed_len=   1.665 m, corr_signed=   1.610 m, abs_len=   1.665 m, valid=55, dut_min=  -8.02 dB, ref_min=  -4.57 dB, resid=  4.57 deg, OK

Stage summary:
  stage                = dut_forward
  runs                 = 5
  mean tau             = 8.451 ns
  mean signed length   = 1.672 m
  corrected signed len = 1.617 m
  signed length span   = 0.063 m
  corrected span       = 0.063 m
  min valid tones      = 55
  max residual         = 4.76 deg
  verdict              = OK
TX output is OFF. You may change hardware before the next test prompt.

========================================================================
DUT cable reversed
========================================================================
- Reverse the same RJ45 cable end-to-end on the DUT path.
- Keep TX1A/RX1A and TX2A/RX2A channel roles unchanged.
- This checks connector/fixture direction sensitivity.


Ready for test: DUT cable reversed
TX output is OFF. change hardware now, then keep your hands off the wiring.
Press Enter to START this test...
TX multitone ON
dut_reversed         run 1: tau=    8.401 ns, signed_len=   1.662 m, corr_signed=   1.607 m, abs_len=   1.662 m, valid=55, dut_min=  -7.71 dB, ref_min=  -4.56 dB, resid=  4.59 deg, OK
dut_reversed         run 2: tau=    8.302 ns, signed_len=   1.643 m, corr_signed=   1.588 m, abs_len=   1.643 m, valid=55, dut_min=  -7.59 dB, ref_min=  -4.56 dB, resid=  4.56 deg, OK
dut_reversed         run 3: tau=    8.253 ns, signed_len=   1.633 m, corr_signed=   1.578 m, abs_len=   1.633 m, valid=55, dut_min=  -7.66 dB, ref_min=  -4.55 dB, resid=  4.56 deg, OK
dut_reversed         run 4: tau=    8.183 ns, signed_len=   1.619 m, corr_signed=   1.564 m, abs_len=   1.619 m, valid=55, dut_min=  -7.75 dB, ref_min=  -4.59 dB, resid=  4.53 deg, OK
dut_reversed         run 5: tau=    8.201 ns, signed_len=   1.623 m, corr_signed=   1.568 m, abs_len=   1.623 m, valid=55, dut_min=  -7.84 dB, ref_min=  -4.57 dB, resid=  4.53 deg, OK

Stage summary:
  stage                = dut_reversed
  runs                 = 5
  mean tau             = 8.268 ns
  mean signed length   = 1.636 m
  corrected signed len = 1.581 m
  signed length span   = 0.043 m
  corrected span       = 0.043 m
  min valid tones      = 55
  max residual         = 4.59 deg
  verdict              = OK
TX output is OFF. You may change hardware before the next test prompt.

## 7.91m 长线

### 正

========================================================================
DUT cable forward
========================================================================
- DUT path: TX1A -> SMA/RJ45 fixture -> cable -> SMA/RJ45 fixture -> RX1A
- Reference path: TX2A -> RX2A with the chosen reference SMA cable.
- Keep the reference SMA cable fixed for all DUT cable runs.


Ready for test: DUT cable forward
TX output is OFF. change hardware now, then keep your hands off the wiring.
Press Enter to START this test...
TX multitone ON
dut_forward          run 1: tau=   41.583 ns, signed_len=   8.228 m, corr_signed=   8.173 m, abs_len=   8.228 m, valid=55, dut_min= -16.73 dB, ref_min=  -4.76 dB, resid=  2.49 deg, OK
dut_forward          run 2: tau=   41.476 ns, signed_len=   8.207 m, corr_signed=   8.152 m, abs_len=   8.207 m, valid=55, dut_min= -17.35 dB, ref_min=  -4.77 dB, resid=  2.22 deg, OK
dut_forward          run 3: tau=   41.628 ns, signed_len=   8.237 m, corr_signed=   8.182 m, abs_len=   8.237 m, valid=55, dut_min= -17.37 dB, ref_min=  -4.74 dB, resid=  2.03 deg, OK
dut_forward          run 4: tau=   41.570 ns, signed_len=   8.225 m, corr_signed=   8.170 m, abs_len=   8.225 m, valid=55, dut_min= -17.42 dB, ref_min=  -4.76 dB, resid=  2.06 deg, OK
dut_forward          run 5: tau=   41.509 ns, signed_len=   8.213 m, corr_signed=   8.158 m, abs_len=   8.213 m, valid=55, dut_min= -17.51 dB, ref_min=  -4.77 dB, resid=  2.11 deg, OK

Stage summary:
  stage                = dut_forward
  runs                 = 5
  mean tau             = 41.553 ns
  mean signed length   = 8.222 m
  corrected signed len = 8.167 m
  signed length span   = 0.030 m
  corrected span       = 0.030 m
  min valid tones      = 55
  max residual         = 2.49 deg
  verdict              = OK
TX output is OFF. You may change hardware before the next test prompt.

========================================================================
DUT cable reversed
========================================================================
- Reverse the same RJ45 cable end-to-end on the DUT path.
- Keep TX1A/RX1A and TX2A/RX2A channel roles unchanged.
- This checks connector/fixture direction sensitivity.


Ready for test: DUT cable reversed
TX output is OFF. change hardware now, then keep your hands off the wiring.
Press Enter to START this test...
TX multitone ON
dut_reversed         run 1: tau=   41.661 ns, signed_len=   8.243 m, corr_signed=   8.188 m, abs_len=   8.243 m, valid=55, dut_min= -16.53 dB, ref_min=  -4.76 dB, resid=  2.03 deg, OK
dut_reversed         run 2: tau=   41.688 ns, signed_len=   8.249 m, corr_signed=   8.194 m, abs_len=   8.249 m, valid=55, dut_min= -16.64 dB, ref_min=  -4.75 dB, resid=  1.91 deg, OK
dut_reversed         run 3: tau=   41.701 ns, signed_len=   8.251 m, corr_signed=   8.196 m, abs_len=   8.251 m, valid=55, dut_min= -16.64 dB, ref_min=  -4.73 dB, resid=  1.89 deg, OK

Stage summary:
  stage                = dut_reversed
  runs                 = 5
  mean tau             = 41.651 ns
  mean signed length   = 8.241 m
  corrected signed len = 8.186 m
  signed length span   = 0.024 m
  corrected span       = 0.024 m
  min valid tones      = 55
  max residual         = 2.06 deg
  verdict              = OK



### 反

DUT cable forward
========================================================================
- DUT path: TX1A -> SMA/RJ45 fixture -> cable -> SMA/RJ45 fixture -> RX1A
- Reference path: TX2A -> RX2A with the chosen reference SMA cable.
- Keep the reference SMA cable fixed for all DUT cable runs.


Ready for test: DUT cable forward
TX output is OFF. change hardware now, then keep your hands off the wiring.
Press Enter to START this test...
TX multitone ON
dut_forward          run 1: tau=   41.229 ns, signed_len=   8.158 m, corr_signed=   8.103 m, abs_len=   8.158 m, valid=55, dut_min= -17.34 dB, ref_min=  -3.50 dB, resid=  1.90 deg, OK
dut_forward          run 2: tau=   41.228 ns, signed_len=   8.158 m, corr_signed=   8.103 m, abs_len=   8.158 m, valid=55, dut_min= -17.27 dB, ref_min=  -3.50 dB, resid=  1.79 deg, OK
dut_forward          run 3: tau=   41.240 ns, signed_len=   8.160 m, corr_signed=   8.105 m, abs_len=   8.160 m, valid=55, dut_min= -17.24 dB, ref_min=  -3.49 dB, resid=  1.89 deg, OK
dut_forward          run 4: tau=   41.256 ns, signed_len=   8.163 m, corr_signed=   8.108 m, abs_len=   8.163 m, valid=55, dut_min= -17.22 dB, ref_min=  -3.48 dB, resid=  1.76 deg, OK
dut_forward          run 5: tau=   41.229 ns, signed_len=   8.158 m, corr_signed=   8.103 m, abs_len=   8.158 m, valid=55, dut_min= -17.29 dB, ref_min=  -3.48 dB, resid=  1.83 deg, OK

Stage summary:
  stage                = dut_forward
  runs                 = 5
  mean tau             = 41.236 ns
  mean signed length   = 8.159 m
  corrected signed len = 8.104 m
  signed length span   = 0.005 m
  corrected span       = 0.005 m
  min valid tones      = 55
  max residual         = 1.90 deg
  verdict              = OK
TX output is OFF. You may change hardware before the next test prompt.

========================================================================
DUT cable reversed
========================================================================
- Reverse the same RJ45 cable end-to-end on the DUT path.
- Keep TX1A/RX1A and TX2A/RX2A channel roles unchanged.
- This checks connector/fixture direction sensitivity.


Ready for test: DUT cable reversed
TX output is OFF. change hardware now, then keep your hands off the wiring.
Press Enter to START this test...
TX multitone ON
dut_reversed         run 1: tau=   41.373 ns, signed_len=   8.186 m, corr_signed=   8.131 m, abs_len=   8.186 m, valid=55, dut_min= -17.31 dB, ref_min=  -3.49 dB, resid=  1.87 deg, OK
dut_reversed         run 2: tau=   41.385 ns, signed_len=   8.189 m, corr_signed=   8.134 m, abs_len=   8.189 m, valid=55, dut_min= -17.38 dB, ref_min=  -3.51 dB, resid=  1.86 deg, OK
dut_reversed         run 3: tau=   41.339 ns, signed_len=   8.180 m, corr_signed=   8.125 m, abs_len=   8.180 m, valid=55, dut_min= -17.47 dB, ref_min=  -3.51 dB, resid=  1.87 deg, OK
dut_reversed         run 4: tau=   41.357 ns, signed_len=   8.183 m, corr_signed=   8.128 m, abs_len=   8.183 m, valid=55, dut_min= -17.46 dB, ref_min=  -3.51 dB, resid=  1.86 deg, OK
dut_reversed         run 5: tau=   41.384 ns, signed_len=   8.188 m, corr_signed=   8.133 m, abs_len=   8.188 m, valid=55, dut_min= -17.41 dB, ref_min=  -3.48 dB, resid=  1.89 deg, OK

Stage summary:
  stage                = dut_reversed
  runs                 = 5
  mean tau             = 41.368 ns
  mean signed length   = 8.185 m
  corrected signed len = 8.130 m
  signed length span   = 0.009 m
  corrected span       = 0.009 m
  min valid tones      = 55
  max residual         = 1.89 deg
  verdict              = OK
TX output is OFF. You may change hardware before the next test prompt.