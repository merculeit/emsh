// SPDX-License-Identifier: BSL-1.0

// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "ctlseq.h"
#include <assert.h>

ctlseq_ev_t ctlseq_sm(ctlseq_st_t *p_st, int c, int *p_psep)
{
	ctlseq_ev_t ev = CTLSEQ_EV_NONE;
	int psep = 0;

	assert(p_st != NULL);

	switch (*p_st)
	{
	case CTLSEQ_ST_FINAL:
		*p_st = CTLSEQ_ST_INIT; // reset
		/* FALLTHROUGH */

	case CTLSEQ_ST_INIT:
		if (c == CTLSEQ_C_CSI_1)
		{
			*p_st = CTLSEQ_ST_ESC;
			ev = CTLSEQ_EV_ESC;
		}
		break;

	case CTLSEQ_ST_ESC:
		if (c == CTLSEQ_C_CSI_2)
		{
			*p_st = CTLSEQ_ST_CSI;
			ev = CTLSEQ_EV_CSI;
		}
		else
		{
			*p_st = CTLSEQ_ST_INIT;
			ev = CTLSEQ_EV_ILSEQ;
		}
		break;

	case CTLSEQ_ST_CSI:
		if (ctlseq_is_param_byte(c))
		{
			*p_st = CTLSEQ_ST_PARAM;
			ev = CTLSEQ_EV_PARAM;

			if (c == CTLSEQ_K_PSEP)
			{
				psep = 1; // an empty parameter sub-string before the first separator
			}
		}
		else if (ctlseq_is_interm_byte(c))
		{
			*p_st = CTLSEQ_ST_INTERM;
			ev = CTLSEQ_EV_INTERM;
		}
		else if (ctlseq_is_final_byte(c))
		{
			*p_st = CTLSEQ_ST_FINAL;
			ev = CTLSEQ_EV_FINAL;
		}
		else
		{
			*p_st = CTLSEQ_ST_INIT;
			ev = CTLSEQ_EV_ILSEQ;
		}
		break;

	case CTLSEQ_ST_PARAM:
		if (ctlseq_is_param_byte(c))
		{
			if (c == CTLSEQ_K_PSEP)
			{
				psep = 1;
			}
		}
		else if (ctlseq_is_interm_byte(c))
		{
			*p_st = CTLSEQ_ST_INTERM;
			ev = CTLSEQ_EV_INTERM;

			psep = 1; // implicitly terminated
		}
		else if (ctlseq_is_final_byte(c))
		{
			*p_st = CTLSEQ_ST_FINAL;
			ev = CTLSEQ_EV_FINAL;

			psep = 1; // implicitly terminated
		}
		else
		{
			*p_st = CTLSEQ_ST_INIT;
			ev = CTLSEQ_EV_ILSEQ;

			psep = 1; // implicitly terminated
		}
		break;

	case CTLSEQ_ST_INTERM:
		if (ctlseq_is_interm_byte(c))
		{
			// nothing to do
		}
		else if (ctlseq_is_final_byte(c))
		{
			*p_st = CTLSEQ_ST_FINAL;
			ev = CTLSEQ_EV_FINAL;
		}
		else
		{
			*p_st = CTLSEQ_ST_INIT;
			ev = CTLSEQ_EV_ILSEQ;
		}
		break;
	}

	if (p_psep != NULL)
	{
		*p_psep = psep;
	}
	return ev;
}
