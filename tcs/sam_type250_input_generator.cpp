#define _TCSTYPEINTERFACE_
#include "tcstype.h"

using namespace std;

enum {
	P_I_B,
	P_T_DB,
	P_V_WIND,
	P_P_AMB,
	P_T_DP,
	P_T_COLD_IN,
	P_M_DOT_IN,
	P_DEFOCUS,

	O_I_B,
	O_T_DB,
	O_V_WIND,
	O_P_AMB,
	O_T_DP,
	O_T_COLD_IN,
	O_M_DOT_IN,
	O_DEFOCUS,

	N_MAX
};

tcsvarinfo sam_type250_input_generator_variables[] = {
	/* DIRECTION    DATATYPE      INDEX       NAME           LABEL                                  UNITS      GROUP    META    DEFAULTVALUE */
	{ TCS_PARAM, TCS_ARRAY,  P_I_B,      "I_b",       "Direct normal incident solar irradiation",   "W/m^2",   "",  "",  "" },
	{ TCS_PARAM, TCS_ARRAY,  P_T_DB,     "T_db",      "Dry bulb temperature",                       "C",       "",  "",  "" },
	{ TCS_PARAM, TCS_ARRAY,  P_V_WIND,   "V_wind",    "Wind speed",                                 "m/s",     "",  "",  "" },
	{ TCS_PARAM, TCS_ARRAY,  P_P_AMB,    "P_amb",     "Ambient pressure",                           "mbar",    "",  "",  "" },
	{ TCS_PARAM, TCS_ARRAY,  P_T_DP,     "T_dp",      "Dew point temperature",                      "C",       "",  "",  "" },
	{ TCS_PARAM, TCS_ARRAY,  P_T_COLD_IN,"T_cold_in", "HTF inlet temperature",                      "C",       "",  "",  "" },
	{ TCS_PARAM, TCS_ARRAY,  P_M_DOT_IN, "m_dot_in",  "HTF mass flow rate at inlet",                "kg/hr",   "",  "",  "" },
	{ TCS_PARAM, TCS_ARRAY,  P_DEFOCUS,  "defocus",   "Mirror defocus fraction",                    "",        "",  "",  "" },

	{ TCS_OUTPUT, TCS_NUMBER,  O_I_B,      "O_I_b",       "Direct normal incident solar irradiation",   "W/m^2",   "",  "",  "" },
	{ TCS_OUTPUT, TCS_NUMBER,  O_T_DB,     "O_T_db",      "Dry bulb temperature",                       "C",       "",  "",  "" },
	{ TCS_OUTPUT, TCS_NUMBER,  O_V_WIND,   "O_V_wind",    "Wind speed",                                 "m/s",     "",  "",  "" },
	{ TCS_OUTPUT, TCS_NUMBER,  O_P_AMB,    "O_P_amb",     "Ambient pressure",                           "mbar",    "",  "",  "" },
	{ TCS_OUTPUT, TCS_NUMBER,  O_T_DP,     "O_T_dp",      "Dew point temperature",                      "C",       "",  "",  "" },
	{ TCS_OUTPUT, TCS_NUMBER,  O_T_COLD_IN,"O_T_cold_in", "HTF inlet temperature",                      "C",       "",  "",  "" },
	{ TCS_OUTPUT, TCS_NUMBER,  O_M_DOT_IN, "O_m_dot_in",  "HTF mass flow rate at inlet",                "kg/hr",   "",  "",  "" },
	{ TCS_OUTPUT, TCS_NUMBER,  O_DEFOCUS,  "O_defocus",   "Mirror defocus fraction",                    "",        "",  "",  "" },

	{ TCS_INVALID, TCS_INVALID, N_MAX, 0, 0, 0, 0, 0, 0 }
};

class sam_type250_input_generator : public tcstypeinterface
{
private:
	// DNI
	double * I_b;
	int nval_I_b;

	// Dry bulb temperature
	double * T_db;
	int nval_T_db;

	// Wind speed
	double * V_wind;
	int nval_V_wind;

	// Ambient pressure
	double * P_amb;
	int nval_P_amb;

	// Dew point temperature
	double * T_dp;
	int nval_T_dp;

	// HTF inlet temperature
	double * T_cold_in;
	int nval_T_cold_in;

	// HTF inlet mass flow rate
	double * m_dot_in;
	int nval_m_dot_in;

	// Defocus
	double * defocus;
	int nval_defocus;

	// Index counter
	int nval_current;

public:

	sam_type250_input_generator(tcscontext *cxt, tcstypeinfo *ti)
		: tcstypeinterface(cxt, ti)
	{
		// DNI
		I_b = NULL;
		nval_I_b = -1;

		// Dry bulb temperature
		T_db = NULL;
		nval_T_db = -1;

		// Wind speed
		V_wind = NULL;
		nval_V_wind = -1;

		// Ambient pressure
		P_amb = NULL;
		nval_P_amb = -1;

		// Dew point temperature
		T_dp = NULL;
		nval_T_dp = -1;

		// HTF inlet temperature
		T_cold_in = NULL;
		nval_T_cold_in = -1;

		// HTF inlet mass flow rate
		m_dot_in = NULL;
		nval_m_dot_in = -1;

		// Defocus
		defocus = NULL;
		nval_defocus = -1;

		nval_current = 1;
	}

	virtual ~sam_type250_input_generator() {}

	virtual int init()
	{
		
		I_b = value(P_I_B, &nval_I_b);			//[W/m^2] DNI

		T_db = value(P_T_DB, &nval_T_db);		//[C] Dry bulb temperature

		V_wind = value(P_V_WIND, &nval_V_wind);	//[m/s] Wind speed

		P_amb = value(P_P_AMB, &nval_P_amb);	//[mbar] Ambient pressure

		T_dp = value(P_T_DP, &nval_T_dp);		//[C] Dew point temperature

		T_cold_in = value(P_T_COLD_IN, &nval_T_cold_in);	//[C] HTF inlet temperature

		m_dot_in = value(P_M_DOT_IN, &nval_m_dot_in);		//[kg/hr] HTF inlet mass flow rate

		defocus = value(P_M_DOT_IN, &nval_defocus);			//[-] Defocus

		if( nval_I_b != nval_T_db || nval_T_db != nval_V_wind || nval_V_wind != nval_P_amb || nval_P_amb != nval_T_dp || nval_T_dp != nval_T_cold_in || nval_T_cold_in != nval_m_dot_in || nval_m_dot_in != nval_defocus )
		{
			message("All parameters arrays must be the same length");
			return -1;
		}

		if( nval_I_b < 1 )
		{
			message("Parameter arrays must have at least 1 value");
			return -1;
		}

		return 0;
	}

	virtual int call(double time, double step, int ncall)
	{
		if(nval_current > nval_I_b)
		{
			message("The simulation is running simulation %d. The length of the parameter arrays is %d.", nval_current, nval_I_b);
			return -1;
		}

		value( O_I_B, I_b[nval_current] );      
		value( O_T_DB, T_db[nval_current] );    
		value( O_V_WIND, V_wind[nval_current] );  
		value( O_P_AMB, P_amb[nval_current] );
		value( O_T_DP, T_dp[nval_current] );
		value( O_T_COLD_IN, T_cold_in[nval_current] );
		value( O_M_DOT_IN, m_dot_in[nval_current] );
		value( O_DEFOCUS, defocus[nval_current] );

		return 0;
	}

	virtual int converged(double time)
	{
		nval_current++;		// Advanced index counter

		return 0;
	}

};

TCS_IMPLEMENT_TYPE(sam_type250_input_generator, "Input generator for Type250", "Ty Neises", 1, sam_type250_input_generator_variables, NULL, 1)