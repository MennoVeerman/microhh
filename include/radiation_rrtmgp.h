/*
 * MicroHH
 * Copyright (c) 2011-2020 Chiel van Heerwaarden
 * Copyright (c) 2011-2020 Thijs Heus
 * Copyright (c) 2014-2020 Bart van Stratum
 *
 * This file is part of MicroHH
 *
 * MicroHH is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * MicroHH is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with MicroHH.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RADIATION_RRTMGP_H
#define RADIATION_RRTMGP_H

#include "radiation.h"
#include "field3d_operators.h"

#include "Gas_concs.h"
#include "Gas_optics_rrtmgp.h"
#include "Source_functions.h"
#include "Cloud_optics.h"

class Master;
class Input;
template<typename> class Grid;
template<typename> class Stats;
template<typename> class Diff;
template<typename> class Column;
template<typename> class Dump;
template<typename> class Cross;
template<typename> class Field3d;
template<typename> class Thermo;
template<typename> class Timeloop;

template<typename TF>
class Radiation_rrtmgp : public Radiation<TF>
{
	public:
		Radiation_rrtmgp(Master&, Grid<TF>&, Fields<TF>&, Input&);
        virtual ~Radiation_rrtmgp() {}
        bool check_field_exists(std::string name)
        { throw std::runtime_error("\"check_field_exists()\" is not implemented in radiation_rrtmpg"); }

        void init(Timeloop<TF>&);
        void create(
                Input&, Netcdf_handle&, Thermo<TF>&,
                Stats<TF>&, Column<TF>&, Cross<TF>&, Dump<TF>&);
        void exec(Thermo<TF>&, double, Timeloop<TF>&, Stats<TF>&);

        unsigned long get_time_limit(unsigned long);

        void get_radiation_field(Field3d<TF>&, std::string, Thermo<TF>&, Timeloop<TF>&)
        { throw std::runtime_error("\"get_radiation_field()\" is not implemented in radiation_rrtmpg"); }
        std::vector<TF>& get_surface_radiation(std::string);

        void exec_all_stats(
                Stats<TF>&, Cross<TF>&, Dump<TF>&, Column<TF>&,
                Thermo<TF>&, Timeloop<TF>&, const unsigned long, const int);

        void exec_column(Column<TF>&, Thermo<TF>&, Timeloop<TF>&) {};

	private:
		using Radiation<TF>::swradiation;
		using Radiation<TF>::master;
		using Radiation<TF>::grid;
		using Radiation<TF>::fields;
		using Radiation<TF>::field3d_operators;

        void create_column(
                Input&, Netcdf_handle&, Thermo<TF>&, Stats<TF>&);
        void create_column_longwave(
                Input&, Netcdf_handle&, Thermo<TF>&, Stats<TF>&,
                const Gas_concs<double>&);
        void create_column_shortwave(
                Input&, Netcdf_handle&, Thermo<TF>&, Stats<TF>&,
                const Gas_concs<double>&);
        void create_dump(Dump<TF>&);    

        void read_background_profiles(
                Netcdf_handle&, const Gas_concs<double>&);

        void create_solver(
                Input&, Netcdf_handle&, Thermo<TF>&, Stats<TF>&, Column<TF>&);
        void create_solver_longwave(
                Input&, Netcdf_handle&, Thermo<TF>&, Stats<TF>&, Column<TF>&,
                const Gas_concs<double>&);
        void create_solver_shortwave(
                Input&, Netcdf_handle&, Thermo<TF>&, Stats<TF>&, Column<TF>&,
                const Gas_concs<double>&);

        void exec_longwave(
                Thermo<TF>&, Timeloop<TF>&, Stats<TF>&,
                Array<double,2>&, Array<double,2>&, Array<double,2>&,
                const Array<double,2>&, const Array<double,2>&, const Array<double,1>&,
                const Array<double,2>&, const Array<double,2>&, const Array<double,2>&,
                const bool);

        void exec_shortwave(
                Thermo<TF>&, Timeloop<TF>&, Stats<TF>&,
                Array<double,2>&, Array<double,2>&, Array<double,2>&, Array<double,2>&,
                const Array<double,2>&, const Array<double,2>&,
                const Array<double,2>&, const Array<double,2>&, const Array<double,2>&,
                const bool);

        bool is_day(double);  // Switch between day/night, based on sza

        // void exec_stats(Stats<TF>&, Thermo<TF>&, Timeloop<TF>&);
        // void exec_cross(Cross<TF>&, const int, Thermo<TF>&, Timeloop<TF>&);
        // void exec_dump(Dump<TF>&, const int, Thermo<TF>&, Timeloop<TF>&) {};

        const std::string tend_name = "rad";
        const std::string tend_longname = "Radiation";

        bool sw_longwave;
        bool sw_shortwave;
        bool sw_clear_sky_stats;
        bool sw_fixed_sza;

        double dt_rad_sw;
        double dt_rad_lw;
        unsigned long idt_rad_sw;
        unsigned long idt_rad_lw;

        std::vector<std::string> crosslist;
        std::vector<std::string> dumplist;

        // RRTMGP related variables.
        double tsi_scaling; // Total solar irradiance scaling factor.
        double t_sfc;       // Surface absolute temperature in K.
        double emis_sfc;    // Surface emissivity.
        double sfc_alb_dir; // Surface albedo.
        double sfc_alb_dif; // Surface albedo for diffuse light.
        double mu0;         // Cosine of solar zenith angle.
        double Nc0;         // Total droplet number concentration.

        TF lat;    // Latitude (degrees)
        TF lon;    // Longitude (degrees)

        // The reference column for the full profile.
        Array<double,2> lw_flux_dn_inc;
        Array<double,2> sw_flux_dn_dir_inc;
        Array<double,2> sw_flux_dn_dif_inc;

        int n_col;
        int n_lay_col;
        int n_lev_col;

        Array<double,2> p_lay_col;
        Array<double,2> t_lay_col;
        Array<double,2> p_lev_col;
        Array<double,2> t_lev_col;
        Array<double,2> col_dry;

        // Fluxes of reference column
        Array<double,2> lw_flux_up_col;
        Array<double,2> lw_flux_dn_col;
        Array<double,2> lw_flux_net_col;

        Array<double,2> sw_flux_up_col;
        Array<double,2> sw_flux_dn_col;
        Array<double,2> sw_flux_dn_dir_col;
        Array<double,2> sw_flux_net_col;

        Gas_concs<double> gas_concs_col;

        std::unique_ptr<Source_func_lw<double>> sources_lw;
        std::unique_ptr<Optical_props_arry<double>> optical_props_lw;
        std::unique_ptr<Optical_props_arry<double>> optical_props_sw;

        // The full solver.
        Gas_concs<double> gas_concs;
        std::unique_ptr<Gas_optics_rrtmgp<double>> kdist_lw;
        std::unique_ptr<Gas_optics_rrtmgp<double>> kdist_sw;

        std::unique_ptr<Cloud_optics<double>> cloud_lw;
        std::unique_ptr<Cloud_optics<double>> cloud_sw;

        // Surface radiative fluxes
        std::vector<TF> lw_flux_dn_sfc;
        std::vector<TF> lw_flux_up_sfc;

        std::vector<TF> sw_flux_dn_sfc;
        std::vector<TF> sw_flux_up_sfc;

};
#endif
