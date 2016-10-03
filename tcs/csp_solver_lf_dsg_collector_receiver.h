#ifndef __csp_solver_lf_dsg_collector_receiver_
#define __csp_solver_lf_dsg_collector_receiver_

#include "csp_solver_core.h"
#include "htf_props.h"
#include "tcstype.h"

#include "lib_weatherfile.h"
#include <cmath>
#include "sam_csp_util.h"
#include "water_properties.h"

#include "numeric_solvers.h"

#include <iostream>
#include <fstream>

class C_csp_lf_dsg_collector_receiver : public C_csp_collector_receiver
{
private:

	// *******************************************
	// Hardcoded member data
		// Timestep management
	double m_step_recirc;		//[s] 
		// Conversion constants
	double m_d2r;				//[deg/rad] Degree-to-radian conversion
	double m_r2d;				//[rad/deg] Radian-to-degree conversion
	double m_mtoinch;			//[m/in] meter-to-inch conversion
		// DSG Model Constants
	double m_P_max;				//[bar]
	double m_fP_turb_min;		//[-] Minimum fractional operating pressure (of design) at the turbine inlet
		// Water props limits
	double m_wp_max_temp;		//[K]
	double m_wp_min_temp;		//[K]
	double m_wp_max_pres;		//[kPa]
	double m_wp_min_pres;		//[kPa]
	// *******************************************
	// *******************************************

	// *******************************************
	// Design Calculations
		// Geometry and Layout
	util::matrix_t<double> m_D_h;		//[m]
	util::matrix_t<double> m_A_cs;		//[m2]
	util::matrix_t<double> m_EPSILON_5;	//[-]
	util::matrix_t<double> m_eta_opt_fixed;
	util::matrix_t<double> m_opteff_des;
	double m_C_thermal;			//[kJ/K] Thermal capacity per receiver
	double m_fP_sf_tot;			//[-] Total fractional pressure drop across the solar field
	int m_n_rows_matrix;		//[-] 1 if Single Geom, 2 if Multigeom
	int m_nModTot;				//[-] nBoiler + nSH
	bool m_is_sh;				//[-]
	double m_Ap_tot;			//[m2] Total solar field aperture area	
		// Energy and mass balance calcs
	double m_q_dot_abs_tot_des;	//[kWt] SYSTEM total thermal power absorbed by steam at design
	double m_m_dot_min;			//[kg/s] LOOP min mass flow rate - max of field & PC bases
	double m_m_dot_max;			//[kg/s] LOOP max mass flow rate - min of field & PC bases
	double m_m_dot_b_max;		//[kg/s] LOOP max mass flow rate through boiler (m_m_dot_max/x_b_des)
	double m_m_dot_b_des;		//[kg/s] SYSTEM mass flow rate at power cycle design
	double m_m_dot_pb_des;		//[kg/s] SYSTEM mass flow rate at power cycle design
	double m_m_dot_des;			//[kg/s] SYSTEM design point mass flow rate - field design basis
	double m_m_dot_tot;			//[kg/s] SYSTEM mass flow rate off-design
	// *******************************************
	// *******************************************

	// *******************************************
	// Timestep Calculations
		// Control & operation
	int m_operating_mode_converged;
	int m_operating_mode;
	int m_ncall;
		// *********************************************
		// CSP Solver Temperature Tracking
		// *********************************************
			// Temperatures from the most recent converged() operation
	C_csp_solver_steam_state mc_sys_cold_out_t_end_converged;			// End-of-previously-converged-timestep outlet condition from cold system/header/field
	std::vector<C_csp_solver_steam_state> mc_sca_out_t_end_converged;	// End-of-previously-converged-timestep outlet condition from each SCA in loop
	C_csp_solver_steam_state mc_sys_hot_out_t_end_converged;			// End-of-previously-converged-timestep outlet condition hot cold system/header/field
			// Temperature from the most recent timestep (in the event that a method solves multiple, shorter timesteps
	C_csp_solver_steam_state mc_sys_cold_out_t_end_last;			// End-of-previous-timestep outlet condition from cold system/header/field
	std::vector<C_csp_solver_steam_state> mc_sca_out_t_end_last;	// End-of-previous-timestep outlet condition from each SCA in loop
	C_csp_solver_steam_state mc_sys_hot_out_t_end_last;				// End-of-previous-timestep outlet condition hot cold system/header/field

			// Latest temperature solved in loop_energy_balance
	C_csp_solver_steam_state mc_sys_cold_in_t_int;		// Time-integrated inlet condition to cold system/header/field
	C_csp_solver_steam_state mc_sys_cold_out_t_end;		// End-of-timestep outlet condition from cold system/header/field
	C_csp_solver_steam_state mc_sys_cold_out_t_int;		// Time-integrated outlet condition from cold system/header/field
	
	std::vector<C_csp_solver_steam_state> mc_sca_in_t_int;     // Time-integrated inlet condition to each SCA in loop
	std::vector<C_csp_solver_steam_state> mc_sca_out_t_end;	   // End-of-timestep outlet condition from each SCA in loop
	std::vector<C_csp_solver_steam_state> mc_sca_out_t_int;	   // Time-integrated outlet condition from each SCA in loop
	
	C_csp_solver_steam_state mc_sys_hot_in_t_int;		// Time-integrated inlet condition to hot system/header/field
	C_csp_solver_steam_state mc_sys_hot_out_t_end;		// End-of-timestep outlet condition hot cold system/header/field
	C_csp_solver_steam_state mc_sys_hot_out_t_int;		// Time-integrated outlet condition hot cold system/header/field

		// *********************************************
		// TCS Shell Stuff State-Point Tracking
		// Hot System/Field Headers Outlet
	C_csp_solver_steam_state mc_sys_h_out;
	std::vector<double> m_T_ave_prev;
	util::matrix_t<double> m_T_ave;
	util::matrix_t<double> m_h_in;
	util::matrix_t<double> m_h_out;
		// *********************************************
		// *********************************************
	// ****************************************************************
		// ****************************************************************
		// Sun Position
	double m_phi_t;
	double m_theta_L;
		// Optical calcs
	double m_ftrack;		//[-] Fraction of timestep that solar field is tracking
		// Energy Balance
	std::vector<double> m_q_inc;		//[kWt] Incident beam radiation for each receiver in loop
	util::matrix_t<double> m_eta_optical;	//[-] Optical efficiency for each collector geometry
	std::vector<double> m_q_rec;			//[kWt] Incident thermal power on receiver after *optical* losses and *defocus*
	std::vector<double> m_q_rec_control_df;	//[kWt] Incident thermal power on receiver after *optical* and CONTROL defocus
	std::vector<double> m_q_loss;		//[kWt] Thermal loss for each receiver in loop
	std::vector<double> m_q_abs;		//[kWt] Thermal power absorbed by steam in each receiver
	
	double m_Q_field_losses_total;		//[MJ] scas + xover + hot_HR + cold_HR
	double m_q_rec_loop;		//[kWt] LOOP total thermal power on receiver after *optical* losses and *defocus*
	double m_q_inc_loop;		//[kWt] LOOP total incident beam radiation

	// *********************************************
	// Required for backwards compatability with TCS - call & init & converged only!
	// *********************************************
	double m_defocus_prev;	//[-]
	double m_t_sby_prev;	//[-]
	double m_t_sby;			//[-]
	bool m_is_pb_on_prev;	//[-]
	bool m_is_pb_on;		//[-]
	double m_T_sys_prev;	//[K]
	double m_defocus;		//[-]
	bool m_is_def;			//[-]
	double m_err_def;		//[-]
	double m_tol_def;		//[-]
	double m_rc;			//[-]
	// *********************************************
	// *********************************************

	int freeze_protection(const C_csp_weatherreader::S_outputs &weather, double P_field_out /*bar*/, 
		double T_cold_in /*K*/, double m_dot_loop /*kg/s*/, double h_sca_out_target /*kJ/kg*/, 
		const C_csp_solver_sim_info &sim_info, double & Q_fp /*MJ*/);

	double od_pressure(double m_dot_loop /*kg/s*/);

	void apply_component_defocus(double defocus /*-*/);

public:

	// Class to save messages for up stream classes
	C_csp_messages mc_csp_messages;

	//parameters and inputs
	// Class Instances
	emit_table eps_abs;
	OpticalDataTable b_optical_table;
	OpticalDataTable sh_optical_table;
	TwoOptTables optical_tables;
	P_max_check check_pressure;
	enth_lim check_h;
	water_state wp;
	Evacuated_Receiver evac_tube_model;
	HTFProperties htfProps;
	
	// Parameters
	double m_q_max_aux;			//[kWt] 
	double m_LHV_eff;			//[-]
	double m_T_set_aux;			//[K]
	double m_T_field_in_des;	//[K]
	double m_T_field_out_des;	//[K]
	double m_x_b_des;			//[-]
	double m_P_turb_des;		//[bar]
	double m_fP_hdr_c;			//[-]
	double m_fP_sf_boil;		//[-]
	double m_fP_boil_to_sh;		//[-]
	double m_fP_sf_sh;			//[-]
	double m_fP_hdr_h;			//[-]
	double m_q_pb_des;			//[kWt]
	double m_W_pb_des;			//[kWe]
	double m_cycle_cutoff_frac;		//[-]
	double m_cycle_max_fraction;	//[-]
	double m_m_dot_min_frac;	//[-]
	double m_m_dot_max_frac;	//[-]
	double m_t_sby_des;			//[hr]
	double m_q_sby_frac;		//[-]
	double m_PB_pump_coef;		//[kW/kg]
	double m_PB_fixed_par;		//[-]
	std::vector<double> m_bop_array;
	std::vector<double> m_aux_array;
	double m_T_startup;			//[K]

	int m_fossil_mode;			//[-]
	double m_I_bn_des;			//[W/m2]
	bool m_is_oncethru;			//[-]
	
	// True = target outlet state is superheat and uses(m_T_field_out_des), False = target outlet state is 2-phase and uses (m_x_b_des)
	bool m_is_sh_target;		//[-] 
	
	bool m_is_multgeom;			//[-]
	int m_nModBoil;				//[-]
	int m_nModSH;				//[-]
	int m_nLoops;				//[-]
	double m_eta_pump;			//[-]
	double m_latitude;			//[rad]
	double m_theta_stow;		//[rad]
	double m_theta_dep;			//[rad]
	double m_T_field_ini;		//[K]
	double m_T_fp;				//[K]
	double m_Pipe_hl_coef;		//[W/m2-K]
	double m_SCA_drives_elec;	//[W/SCA]
	double m_ColAz;				//[rad]
	double m_e_startup;			//[kJ/K-m2], Thermal inertia contribution per sq meter of solar field
	double m_T_amb_des_sf;		//[K]
	double m_V_wind_max;		//[m/s]
	std::vector<double> m_ffrac;

	util::matrix_t<double> m_A_aperture;
	util::matrix_t<double> m_L_col;
	util::matrix_t<double> m_OptCharType;
	util::matrix_t<double> m_IAM_T;
	util::matrix_t<double> m_IAM_L;
	util::matrix_t<double> m_TrackingError;
	util::matrix_t<double> m_GeomEffects;
	util::matrix_t<double> m_rho_mirror_clean;
	util::matrix_t<double> m_dirt_mirror;
	util::matrix_t<double> m_error;
	util::matrix_t<double> m_HLCharType;
	util::matrix_t<double> m_HL_dT;
	util::matrix_t<double> m_HL_W;
	util::matrix_t<double> m_D_2;
	util::matrix_t<double> m_D_3;
	util::matrix_t<double> m_D_4;
	util::matrix_t<double> m_D_5;
	util::matrix_t<double> m_D_p;
	util::matrix_t<double> m_Rough;
	util::matrix_t<double> m_Flow_type;
	util::matrix_t<double> m_AbsorberMaterial_in;
	util::matrix_t<double> m_b_eps_HCE1;
	util::matrix_t<double> m_b_eps_HCE2;
	util::matrix_t<double> m_b_eps_HCE3;
	util::matrix_t<double> m_b_eps_HCE4;
	util::matrix_t<double> m_sh_eps_HCE1;
	util::matrix_t<double> m_sh_eps_HCE2;
	util::matrix_t<double> m_sh_eps_HCE3;
	util::matrix_t<double> m_sh_eps_HCE4;
	util::matrix_t<double> m_HCE_FieldFrac;
	util::matrix_t<double> m_alpha_abs;
	util::matrix_t<double> m_alpha_env;
	util::matrix_t<double> m_EPSILON_4;
	util::matrix_t<double> m_Tau_envelope;
	util::matrix_t<bool> m_GlazingIntactIn;
	util::matrix_t<double> m_AnnulusGas_in;
	util::matrix_t<double> m_P_a;
	util::matrix_t<double> m_Design_loss;
	util::matrix_t<double> m_Shadowing;
	util::matrix_t<double> m_Dirt_HCE;
	util::matrix_t<double> m_b_OpticalTable;
	util::matrix_t<double> m_sh_OpticalTable;	

	// **************************************************************************
	// **************************************************************************
	// **************************************************************************

	C_csp_lf_dsg_collector_receiver();

	~C_csp_lf_dsg_collector_receiver(){};

		// Trunk version
	virtual void init(const C_csp_collector_receiver::S_csp_cr_init_inputs init_inputs,
		C_csp_collector_receiver::S_csp_cr_solved_params & solved_params);

	virtual int get_operating_state();

	virtual double get_startup_time();
	virtual double get_startup_energy(double step /*sec*/); //MWh
	virtual double get_pumping_parasitic_coef();  //MWe/MWt
	virtual double get_min_power_delivery();    //MWt

	virtual void off(const C_csp_weatherreader::S_outputs &weather,
		const C_csp_solver_htf_1state &htf_state_in,
		C_csp_collector_receiver::S_csp_cr_out_solver &cr_out_solver,
		const C_csp_solver_sim_info &sim_info);

	virtual void startup(const C_csp_weatherreader::S_outputs &weather,
		const C_csp_solver_htf_1state &htf_state_in,
		C_csp_collector_receiver::S_csp_cr_out_solver &cr_out_solver,
		const C_csp_solver_sim_info &sim_info);

	virtual void on(const C_csp_weatherreader::S_outputs &weather,
		const C_csp_solver_htf_1state &htf_state_in,
		double field_control,
		C_csp_collector_receiver::S_csp_cr_out_solver &cr_out_solver,
		const C_csp_solver_sim_info &sim_info);

	virtual void estimates(const C_csp_weatherreader::S_outputs &weather,
		const C_csp_solver_htf_1state &htf_state_in,
		C_csp_collector_receiver::S_csp_cr_est_out &est_out,
		const C_csp_solver_sim_info &sim_info);
	
	virtual void converged();

	virtual void write_output_intervals(double report_time_start,
		const std::vector<double> & v_temp_ts_time_end, double report_time_end);

	virtual double calculate_optical_efficiency(const C_csp_weatherreader::S_outputs &weather, const C_csp_solver_sim_info &sim);

	virtual double calculate_thermal_efficiency_approx(const C_csp_weatherreader::S_outputs &weather, double q_incident /*MW*/);

	virtual double get_collector_area();

	// ------------------------------------------ supplemental methods -----------------------------------------------------------
	double turb_pres_frac(double m_dot_nd, int fmode, double ffrac, double fP_min);

	// ------------------------------------------ supplemental methods -----------------------------------------------------------
	class E_piping_config
	{
	public:
		enum
		{
			FIELD = 1,
			LOOP
		};
	};
	class E_loop_energy_balance_exit
	{
	public:
		enum
		{
			SOLVED,
			NaN
		};
	};

	void loop_optical_eta(const C_csp_weatherreader::S_outputs &weather,
		const C_csp_solver_sim_info &sim_info);

	void loop_optical_eta_off();

	void call(const C_csp_weatherreader::S_outputs &weather,
		const C_csp_solver_htf_1state &htf_state_in,
		const C_csp_collector_receiver::S_csp_cr_inputs &inputs,
		C_csp_collector_receiver::S_csp_cr_out_solver &cr_out_solver,
		const C_csp_solver_sim_info &sim_info);

	void transient_energy_bal_numeric_int(double h_in /*kJ/kg*/, double P_in /*kPa*/,
			double q_dot_abs /*kWt*/, double m_dot /*kg/s*/, double T_out_t_end_prev /*K*/, 
			double C_thermal /*kJ/K*/, double step /*s*/, double & h_out_t_end);

	int once_thru_loop_energy_balance_T_t_int(const C_csp_weatherreader::S_outputs &weather,
		double T_cold_in /*K*/, double P_field_out /*bar*/, double m_dot_loop /*kg/s*/, double h_sca_out_target /*kJ/kg*/,
		const C_csp_solver_sim_info &sim_info);

	void update_last_temps();

	void reset_last_temps();

	class C_mono_eq_transient_energy_bal : public C_monotonic_equation
	{
	private:
		water_state mc_wp;

		double m_h_in;		//[kJ/kg]
		double m_P_in;		//[kPa]
		double m_q_dot_abs;	//[kWt]
		double m_m_dot;		//[kg/s]
		double m_T_out_t_end_prev;	//[K]
		double m_C_thermal;	//[kJ/K]
		double m_step;		//[s]

	public:
		C_mono_eq_transient_energy_bal(double h_in /*kJ/kg*/, double P_in /*kPa*/,
			double q_dot_abs /*kWt*/, double m_dot /*kg/s*/, double T_out_t_end_prev /*K*/, 
			double C_thermal /*kJ/K*/, double step /*s*/)
		{
			m_h_in = h_in; m_P_in = P_in; m_q_dot_abs = q_dot_abs; m_m_dot = m_dot;
				m_T_out_t_end_prev = T_out_t_end_prev; m_C_thermal = C_thermal; m_step = step;
		}

		virtual int operator()(double h_out_t_end /*K*/, double *diff_h_out_t_end /*-*/);
	};

	class C_mono_eq_freeze_prot_E_bal : public C_monotonic_equation
	{	// The solver chooses a cold inlet temperature and sends it to the operator. The operator
		//		calls the loop energy balance at the recirculation mass flow rate
		//		and returns the total field heat loss. The solver finds the T_cold_in such that E_fp = E_losses
	private:
		C_csp_lf_dsg_collector_receiver *mpc_dsg_lf;
		C_csp_weatherreader::S_outputs ms_weather;
		double m_P_field_out;	//[bar]
		double m_m_dot_loop;	//[kg/s]
		double m_h_sca_out_target;	//[kJ/kg]
		C_csp_solver_sim_info ms_sim_info;

	public:

		double m_Q_fp;		//[MJ]

		C_mono_eq_freeze_prot_E_bal(C_csp_lf_dsg_collector_receiver *pc_dsg_lf, const C_csp_weatherreader::S_outputs &weather,
						double P_field_out /*bar*/, double m_dot_loop /*kg/s*/, 
						double h_sca_out_target /*kJ/kg*/, const C_csp_solver_sim_info &sim_info)
		{
			mpc_dsg_lf = pc_dsg_lf;
			ms_weather = weather;
			m_P_field_out = P_field_out;	//[bar]
			m_m_dot_loop = m_dot_loop;		//[kg/s]
			m_h_sca_out_target = h_sca_out_target;	//[kJ/kg]
			ms_sim_info = sim_info;

			m_Q_fp = std::numeric_limits<double>::quiet_NaN();	//[MJ]
		}
	
		virtual int operator()(double T_cold_in /*K*/, double *E_loss_balance /*-*/);	
	};

	class C_mono_eq_h_loop_out_target : public C_monotonic_equation
	{
	private:
		C_csp_lf_dsg_collector_receiver *mpc_dsg_lf;
		C_csp_weatherreader::S_outputs ms_weather;
		double m_T_cold_in;		//[K]
		C_csp_solver_sim_info ms_sim_info;

	public:
		C_mono_eq_h_loop_out_target(C_csp_lf_dsg_collector_receiver *pc_dsg_lf, const C_csp_weatherreader::S_outputs &weather,
			double T_cold_in /*K*/, const C_csp_solver_sim_info &sim_info)
		{
			mpc_dsg_lf = pc_dsg_lf;
			ms_weather = weather;
			m_T_cold_in = T_cold_in;		//[K]
			ms_sim_info = sim_info;

			m_P_field_out = std::numeric_limits<double>::quiet_NaN();
			m_h_sca_out_target = std::numeric_limits<double>::quiet_NaN();
		}

		double m_P_field_out;		//[bar]
		double m_h_sca_out_target;	//[kJ/kg]
	
		virtual int operator()(double m_dot_loop /*kg/s*/, double *diff_h_loop_out /*kJ/kg*/);
	};

	class C_mono_eq_defocus : public C_monotonic_equation
	{
	private:
		C_csp_lf_dsg_collector_receiver *mpc_dsg_lf;
		C_csp_weatherreader::S_outputs ms_weather;
		double m_T_cold_in;		//[K]
		double m_P_field_out;	//[bar]
		double m_m_dot_loop;	//[kg/s]
		double m_h_sca_out_target;	//[kJ/kg]
		C_csp_solver_sim_info ms_sim_info;

	public:
		C_mono_eq_defocus(C_csp_lf_dsg_collector_receiver *pc_dsg_lf, const C_csp_weatherreader::S_outputs &weather,
			double T_cold_in /*K*/, double P_field_out /*bar*/, double m_dot_loop /*kg/s*/,
			double h_sca_out_target /*kJ/kg*/, const C_csp_solver_sim_info &sim_info)
		{
			mpc_dsg_lf = pc_dsg_lf;
			ms_weather = weather;
			m_T_cold_in = T_cold_in;		//[K]
			m_P_field_out = P_field_out;	//[bar]
			m_m_dot_loop = m_dot_loop;		//[kg/s]
			m_h_sca_out_target = h_sca_out_target;	//[kJ/kg]
			ms_sim_info = sim_info;
		}
	
		virtual int operator()(double defocus /*-*/, double *diff_h_loop_out /*kJ/kg*/);
	};

};


#endif