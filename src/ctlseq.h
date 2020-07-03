// SPDX-License-Identifier: BSL-1.0

// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#if !defined(CTLSEQ_H_INCLUDED)
#define CTLSEQ_H_INCLUDED

#include <stddef.h>

/*
 * Standard ECMA-48 Fifth Edition
 * See section 5.4 "Control sequences" and its sub-sections for details.
 */

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * - You can extract a control sequence with CTLSEQ_CSI_1, CTLSEQ_CSI_2 and ctlseq_is_final_byte()
 * - You can split a Parameter String with ctlseq_is_param_byte() and CTLSEQ_PSEP
 * - You can determin whether the Parameter String is private or not with ctlseq_is_priv_param_1st_byte()
 * - Intermediate Bytes is up to one byte as long as supporting only standard control functions
 */

// The format of a control sequence is
// CSI P ... P I ... I F

// 5.4 a) CSI is represented by bit combinations 01/11 (reperesenting ESC) and 05/11 in a 7-bit code [...];
#define CTLSEQ_C_CSI_1 0x1B
#define CTLSEQ_C_CSI_2 0x5B

// 5.4 b) P ... P are Parameter Bytes, which, if present, consist of bit combinations from 03/00 to 03/15;
static inline
int ctlseq_is_param_byte(int c)
{
	return 0x30 <= c && c <= 0x3F;
}

// 5.4.1 b) If the first bit combination of the parameter string is in range 03/12 to 03/15, the parameter string is available for private (or experimental) use. [...].
static inline
int ctlseq_is_priv_param_1st_byte(int c)
{
	return 0x3C <= c && c <= 0x3F;
}

// 5.4.2 c) Parameter sub-strings are separated by one bit combination 03/11
#define CTLSEQ_K_PSEP 0x3B

// 5.4.2 g) If the parameter string starts with the bit combination 03/11, an empty parameter sub-string is assumed preceding the separator; if the parameter string terminates with the bit combination 03/11, an empty parameter sub-string is assumed following the separator; if the parameter string contains successive bit combinations 03/11, empty parameter sub-strings are assumed between the separators.

// 5.4 c) I ... I are Intermediate Bytes, which, if present, consist of bit combinations from 02/00 to 02/15;
static inline
int ctlseq_is_interm_byte(int c)
{
	return 0x20 <= c && c <= 0x2F;
}

// 5.4 d) F is the Final Byte; it consist of a bit combination from 04/00 to 07/14; [...].
static inline
int ctlseq_is_final_byte(int c)
{
	return 0x40 <= c && c <= 0x7E;
}

// Table 3 - Final Bytes of control sequences without Intermediate Bytes
#define CTLSEQ_C_ICH  0x40
#define CTLSEQ_C_CUU  0x41
#define CTLSEQ_C_CUD  0x42
#define CTLSEQ_C_CUF  0x43
#define CTLSEQ_C_CUB  0x44
#define CTLSEQ_C_CNL  0x45
#define CTLSEQ_C_CPL  0x46
#define CTLSEQ_C_CHA  0x47
#define CTLSEQ_C_CUP  0x48
#define CTLSEQ_C_CHT  0x49
#define CTLSEQ_C_ED   0x4A
#define CTLSEQ_C_EL   0x4B
#define CTLSEQ_C_IL   0x4C
#define CTLSEQ_C_DL   0x4D
#define CTLSEQ_C_EF   0x4E
#define CTLSEQ_C_EA   0x4F
#define CTLSEQ_C_DCH  0x50
#define CTLSEQ_C_SSE  0x51
#define CTLSEQ_C_CPR  0x52
#define CTLSEQ_C_SU   0x53
#define CTLSEQ_C_SD   0x54
#define CTLSEQ_C_NP   0x55
#define CTLSEQ_C_PP   0x56
#define CTLSEQ_C_CTC  0x57
#define CTLSEQ_C_ECH  0x58
#define CTLSEQ_C_CVT  0x59
#define CTLSEQ_C_CBT  0x5A
#define CTLSEQ_C_SRS  0x5B
#define CTLSEQ_C_PCX  0x5C
#define CTLSEQ_C_SDS  0x5D
#define CTLSEQ_C_SIMD 0x5E
// reserved 0x5F
#define CTLSEQ_C_HPA  0x60
#define CTLSEQ_C_HPR  0x61
#define CTLSEQ_C_REP  0x62
#define CTLSEQ_C_DA   0x63
#define CTLSEQ_C_VPA  0x64
#define CTLSEQ_C_VPR  0x65
#define CTLSEQ_C_HVP  0x66
#define CTLSEQ_C_TBC  0x67
#define CTLSEQ_C_SM   0x68
#define CTLSEQ_C_MC   0x69
#define CTLSEQ_C_HPB  0x6A
#define CTLSEQ_C_VPB  0x6B
#define CTLSEQ_C_RM   0x6C
#define CTLSEQ_C_SGR  0x6D
#define CTLSEQ_C_DSR  0x6E
#define CTLSEQ_C_DAQ  0x6F
// private use 0x70-0x7E

// Table 4 - Final Bytes of control sequences with a single Intermediate Byte 02/00
#define CTLSEQ_C_SL   0x40
#define CTLSEQ_C_SR   0x41
#define CTLSEQ_C_GSM  0x42
#define CTLSEQ_C_GSS  0x43
#define CTLSEQ_C_FNT  0x44
#define CTLSEQ_C_TSS  0x45
#define CTLSEQ_C_JFY  0x46
#define CTLSEQ_C_SPI  0x47
#define CTLSEQ_C_QUAD 0x48
#define CTLSEQ_C_SSU  0x49
#define CTLSEQ_C_PFS  0x4A
#define CTLSEQ_C_SHS  0x4B
#define CTLSEQ_C_SVS  0x4C
#define CTLSEQ_C_IGS  0x4D
// reserved 0x4E
#define CTLSEQ_C_IDCS 0x4F
#define CTLSEQ_C_PPA  0x50
#define CTLSEQ_C_PPR  0x51
#define CTLSEQ_C_PPB  0x52
#define CTLSEQ_C_SPD  0x53
#define CTLSEQ_C_DTA  0x54
#define CTLSEQ_C_SHL  0x55
#define CTLSEQ_C_SLL  0x56
#define CTLSEQ_C_FNK  0x57
#define CTLSEQ_C_SPQR 0x58
#define CTLSEQ_C_SEF  0x59
#define CTLSEQ_C_PEC  0x5A
#define CTLSEQ_C_SSW  0x5B
#define CTLSEQ_C_SACS 0x5C
#define CTLSEQ_C_SAPV 0x5D
#define CTLSEQ_C_STAB 0x5E
#define CTLSEQ_C_GCC  0x5F
#define CTLSEQ_C_TATE 0x60
#define CTLSEQ_C_TALE 0x61
#define CTLSEQ_C_TAC  0x62
#define CTLSEQ_C_TCC  0x63
#define CTLSEQ_C_TSR  0x64
#define CTLSEQ_C_SCO  0x65
#define CTLSEQ_C_SRCS 0x66
#define CTLSEQ_C_SCS  0x67
#define CTLSEQ_C_SLS  0x68
// reserved 0x69
// reserved 0x6A
#define CTLSEQ_C_SCP  0x6B
// reserved 0x6C
// reserved 0x6D
// reserved 0x6E
// reserved 0x6F
// private use 0x70-0x7E

static inline
int ctlseq_is_priv_final_byte(int c)
{
	return 0x70 <= c & c <= 0x7F;
}

#define CTLSEQ_K_MAP_1 0x20

/*
 * Section 5.5 "Independent control functions"
 */

// [...] 2-character escape sequences of the form ESC Fs, where ESC is represented by bit combination 01/11 and Fs is represented by a bit combination from 06/00 to 07/14.
static inline
int ctlseq_is_indep_final_byte(int c)
{
	return 0x60 <= c && c <= 0x7E;
}

// Table 5 - Independent control functions
#define CTLSEQ_C_DMI  0x60
#define CTLSEQ_C_INT  0x61
#define CTLSEQ_C_EMI  0x62
#define CTLSEQ_C_RIS  0x63
#define CTLSEQ_C_CMD  0x64
// reserved 0x65
// reserved 0x66
// reserved 0x67
// reserved 0x68
// reserved 0x69
// reserved 0x6A
// reserved 0x6B
// reserved 0x6C
// reserved 0x6D
#define CTLSEQ_C_LS2  0x6E
#define CTLSEQ_C_LS3  0x6F
// reserved 0x70
// reserved 0x71
// reserved 0x72
// reserved 0x73
// reserved 0x74
// reserved 0x75
// reserved 0x76
// reserved 0x77
// reserved 0x78
// reserved 0x79
// reserved 0x7A
// reserved 0x7B
#define CTLSEQ_C_LS3R  0x7C
#define CTLSEQ_C_LS2R  0x7D
#define CTLSEQ_C_LS1R  0x7E

/*
 * String literal versions
 */

#define CTLSEQ_S_CSI_1 "\x1B"
#define CTLSEQ_S_CSI_2 "\x5B"
#define CTLSEQ_S_CSI   CTLSEQ_S_CSI_1 CTLSEQ_S_CSI_2

#define CTLSEQ_Z_PSEP "\x3B"

#define CTLSEQ_S_ICH  "\x40"
#define CTLSEQ_S_CUU  "\x41"
#define CTLSEQ_S_CUD  "\x42"
#define CTLSEQ_S_CUF  "\x43"
#define CTLSEQ_S_CUB  "\x44"
#define CTLSEQ_S_CNL  "\x45"
#define CTLSEQ_S_CPL  "\x46"
#define CTLSEQ_S_CHA  "\x47"
#define CTLSEQ_S_CUP  "\x48"
#define CTLSEQ_S_CHT  "\x49"
#define CTLSEQ_S_ED   "\x4A"
#define CTLSEQ_S_EL   "\x4B"
#define CTLSEQ_S_IL   "\x4C"
#define CTLSEQ_S_DL   "\x4D"
#define CTLSEQ_S_EF   "\x4E"
#define CTLSEQ_S_EA   "\x4F"
#define CTLSEQ_S_DCH  "\x50"
#define CTLSEQ_S_SSE  "\x51"
#define CTLSEQ_S_CPR  "\x52"
#define CTLSEQ_S_SU   "\x53"
#define CTLSEQ_S_SD   "\x54"
#define CTLSEQ_S_NP   "\x55"
#define CTLSEQ_S_PP   "\x56"
#define CTLSEQ_S_CTC  "\x57"
#define CTLSEQ_S_ECH  "\x58"
#define CTLSEQ_S_CVT  "\x59"
#define CTLSEQ_S_CBT  "\x5A"
#define CTLSEQ_S_SRS  "\x5B"
#define CTLSEQ_S_PCX  "\x5C"
#define CTLSEQ_S_SDS  "\x5D"
#define CTLSEQ_S_SIMD "\x5E"
// reserved 0x5F
#define CTLSEQ_S_HPA  "\x60"
#define CTLSEQ_S_HPR  "\x61"
#define CTLSEQ_S_REP  "\x62"
#define CTLSEQ_S_DA   "\x63"
#define CTLSEQ_S_VPA  "\x64"
#define CTLSEQ_S_VPR  "\x65"
#define CTLSEQ_S_HVP  "\x66"
#define CTLSEQ_S_TBC  "\x67"
#define CTLSEQ_S_SM   "\x68"
#define CTLSEQ_S_MC   "\x69"
#define CTLSEQ_S_HPB  "\x6A"
#define CTLSEQ_S_VPB  "\x6B"
#define CTLSEQ_S_RM   "\x6C"
#define CTLSEQ_S_SGR  "\x6D"
#define CTLSEQ_S_DSR  "\x6E"
#define CTLSEQ_S_DAQ  "\x6F"

#define CTLSEQ_S_SL   "\x40"
#define CTLSEQ_S_SR   "\x41"
#define CTLSEQ_S_GSM  "\x42"
#define CTLSEQ_S_GSS  "\x43"
#define CTLSEQ_S_FNT  "\x44"
#define CTLSEQ_S_TSS  "\x45"
#define CTLSEQ_S_JFY  "\x46"
#define CTLSEQ_S_SPI  "\x47"
#define CTLSEQ_S_QUAD "\x48"
#define CTLSEQ_S_SSU  "\x49"
#define CTLSEQ_S_PFS  "\x4A"
#define CTLSEQ_S_SHS  "\x4B"
#define CTLSEQ_S_SVS  "\x4C"
#define CTLSEQ_S_IGS  "\x4D"
// reserved 0x4E
#define CTLSEQ_S_IDCS "\x4F"
#define CTLSEQ_S_PPA  "\x50"
#define CTLSEQ_S_PPR  "\x51"
#define CTLSEQ_S_PPB  "\x52"
#define CTLSEQ_S_SPD  "\x53"
#define CTLSEQ_S_DTA  "\x54"
#define CTLSEQ_S_SHL  "\x55"
#define CTLSEQ_S_SLL  "\x56"
#define CTLSEQ_S_FNK  "\x57"
#define CTLSEQ_S_SPQR "\x58"
#define CTLSEQ_S_SEF  "\x59"
#define CTLSEQ_S_PEC  "\x5A"
#define CTLSEQ_S_SSW  "\x5B"
#define CTLSEQ_S_SACS "\x5C"
#define CTLSEQ_S_SAPV "\x5D"
#define CTLSEQ_S_STAB "\x5E"
#define CTLSEQ_S_GCC  "\x5F"
#define CTLSEQ_S_TATE "\x60"
#define CTLSEQ_S_TALE "\x61"
#define CTLSEQ_S_TAC  "\x62"
#define CTLSEQ_S_TCC  "\x63"
#define CTLSEQ_S_TSR  "\x64"
#define CTLSEQ_S_SCO  "\x65"
#define CTLSEQ_S_SRCS "\x66"
#define CTLSEQ_S_SCS  "\x67"
#define CTLSEQ_S_SLS  "\x68"
// reserved 0x69
// reserved 0x6A
#define CTLSEQ_S_SCP  "\x6B"
// reserved 0x6C
// reserved 0x6D
// reserved 0x6E
// reserved 0x6F

#define CTLSEQ_Z_MAP_1 "\x20"

#define CTLSEQ_S_DMI  "\x60"
#define CTLSEQ_S_INT  "\x61"
#define CTLSEQ_S_EMI  "\x62"
#define CTLSEQ_S_RIS  "\x63"
#define CTLSEQ_S_CMD  "\x64"
// reserved 0x65
// reserved 0x66
// reserved 0x67
// reserved 0x68
// reserved 0x69
// reserved 0x6A
// reserved 0x6B
// reserved 0x6C
// reserved 0x6D
#define CTLSEQ_S_LS2  "\x6E"
#define CTLSEQ_S_LS3  "\x6F"
// reserved 0x70
// reserved 0x71
// reserved 0x72
// reserved 0x73
// reserved 0x74
// reserved 0x75
// reserved 0x76
// reserved 0x77
// reserved 0x78
// reserved 0x79
// reserved 0x7A
// reserved 0x7B
#define CTLSEQ_S_LS3R  "\x7C"
#define CTLSEQ_S_LS2R  "\x7D"
#define CTLSEQ_S_LS1R  "\x7E"

/*
 * Parser
 */
typedef enum ctlseq_st
{
	CTLSEQ_ST_INIT     = 0, // not within control sequence
	CTLSEQ_ST_ESC      = 1, // at ESC
	CTLSEQ_ST_CSI      = 2, // at CSI
	CTLSEQ_ST_PARAM    = 3, // within parameter bytes
	CTLSEQ_ST_INTERM   = 4, // within intermediate bytes
	CTLSEQ_ST_FINAL    = 5, // at final byte
} ctlseq_st_t;

typedef enum ctlseq_ev
{
	CTLSEQ_EV_NONE     = 0, // no event
	CTLSEQ_EV_ESC      = 1, // start of escape sequence
	CTLSEQ_EV_CSI      = 2, // start of control sequence
	CTLSEQ_EV_PARAM    = 3, // start of parameter bytes
	CTLSEQ_EV_INTERM   = 4, // start of intermediate bytes
	CTLSEQ_EV_FINAL    = 5, // final byte
	CTLSEQ_EV_ILSEQ    = 6, // illegal sequence
} ctlseq_ev_t;

ctlseq_ev_t ctlseq_sm(ctlseq_st_t *p_st, int c, int *p_psep); // state machine (p_psep is nullable)

#if defined(__cplusplus)
}
#endif

#endif // CTLSEQ_H_INCLUDED
