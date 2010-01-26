

// Copyright 2010 by Philipp Kraft
// This file is part of cmf.
//
//   cmf is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 2 of the License, or
//   (at your option) any later version.
//
//   cmf is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with cmf.  If not, see <http://www.gnu.org/licenses/>.
//   
#ifndef ET_h__
#define ET_h__
#include "../../water/flux_connection.h"
#include "../../Atmosphere/Meteorology.h"
#include "../../reach/OpenWaterStorage.h"
#include "../SoilLayer.h"
#include "../Soil/RetentionCurve.h"
#include "ShuttleworthWallace.h"
namespace cmf {
	namespace upslope {
		/// Contains different flux_connection classes for the description of evaporation and transpiration
		namespace ET {
			/// @defgroup ET Evapotranspiration
			/// @ingroup connections

			/// Returns the potential ET after Penman-Monteith using some simplifications for a given Radiation balance, 
			/// aerodynamic and surface resistances, and a vapor pressure deficit
			///
			/// \f[ ET = \frac{\Delta R_n}{\lambda \Delta + \gamma + \gamma \frac{r_s}{r_a}} 
			/// + \frac{c_p\rho_a}{\Delta + \gamma + \gamma \frac{r_s}{r_a}} \frac{e_s - e_a}{r_a} \f]
			/// where
			///  - \f$ ET \f$ is the evapotranspiration in \f$\frac{kg}{m^2 day}\approx \frac{mm}{day}\f$
			///  - \f$ \Delta \left[\frac{kPa}{k}\right]= 4098\ 0.618	 \exp\left(\frac{17.27 T}{T+237.3}\right (T+237.3)^{-2} \f$ is the slope of vapor pressure
			///  - \f$ R_n \left[\frac{MJ}{m^2 day}\right]\f$ is the radiation balance
			///  - \f$ r_s \left[\frac s m\right] \f$ is the surface resistance
			///  - \f$ r_a \left[\frac s m\right] \f$ is the aerodynamic resistance
			///  - \f$ \gamma = 0.067 \left[\frac{kPa}{k}\right] \f$ is the psychrometer constant
			///  - \f$ e_s - e_a \left[kPa\right]\f$ is the vapor pressure deficit
			/// 
			/// @param Rn Radiation balance in \f$ \frac{MJ}{m^2 day} \f$
			/// @param ra Aerodynamic resistance in \f$ \frac s m \f$
			/// @param rs Surface resistance in \f$ \frac s m \f$, is 0 for free water
			/// @param T Actual Temperature in \f$ ^\circ C \f$
			/// @param vap_press_deficit Deficit of vapor pressure \f$ kPa \f$
			real PenmanMonteith(real Rn,real ra,real rs,real T,real vap_press_deficit);
			real PenmanMonteith(cmf::atmosphere::Weather A,const cmf::upslope::vegetation::Vegetation & veg,double h);
			real Tact(real Tpot,const cmf::upslope::SoilLayer & sw,const cmf::upslope::vegetation::Vegetation & veg);
			/// @ingroup ET
			/// A constant evapotranspiration
			class constantETpot : public cmf::water::flux_connection {
			protected:
				std::tr1::weak_ptr<cmf::upslope::SoilLayer> sw;
				
				virtual real calc_q(cmf::math::Time t);
				void NewNodes()
				{
					sw=std::tr1::dynamic_pointer_cast<cmf::upslope::SoilLayer>( left_node());
				}
			public:
				real ETpot_value;
				real GetETpot(cmf::math::Time t) const {return ETpot_value;}
				constantETpot(cmf::upslope::SoilLayer::ptr source,cmf::water::flux_node::ptr ET_target,double constantETpot_value) 
					: flux_connection(source,ET_target,"Constant get_evaporation"),ETpot_value(constantETpot_value)	
				{
					NewNodes();
				}
			};
			/// @ingroup ET
			/// Calculates the potential evapotranspiration according to FAO(1998)
			///
			/// Gouverning equations:
			/// \f{eqnarray*}
			/// \lambda get_evaporation &=& \frac{\Delta\left(R_n - G\right)+\rho_a c_p \frac{e_s - e_a}{r_a}}{\Delta + \gamma\left(1+\frac{r_s}{r_a}\right)} \mbox{ FAO 1998, Eq. 3} \\
			/// \mbox{With:} \\
			/// \Delta &=& 4098 \frac{0.6108 e^{17.27 T}}{(T+237.3)^2} \frac{kPa}{^\circ C} \mbox{	(FAO 1998, Eq. 13): Slope of vapor pressure }	\\
			/// T &=& \mbox{Actual Temperature in } ^\circ C  \\
			/// R_n &=& \mbox{net Radiation (see Atmosphere) in } \frac{MJ}{m^2day}	\\
			/// G &=& 0 \ \frac{MJ}{m^2day} \mbox{ if daily average (FAO 1998, Eq. 42)} \\
			///     && 0.1 R_n \ \mbox{ if day time (FAO 1998, Eq. 45)} \\
			///     && 0.5 R_n \ \mbox{ if night time (FAO 1998, Eq. 46)} \\
			/// \gamma &=& \frac{c_p P}{\epsilon \lambda} \mbox{ (FAO 1998,Eq. 8): Psychrometric constant } \frac{kPa}{^\circ C} \\
			/// c_p &=& 0.001013 \frac{MJ}{kg\ ^\circ C}\mbox{ specific heat at constant pressure } \\
			/// P &=& 101.3 \left(\frac{293-0.0065z}{293}\right)^{5.26}  \mbox{ (FAO 1998,Eq. 7): Mean pressure kPa as a function of elevation above sea level in m} \\
			/// \epsilon &=& 0.622 \mbox{	ratio molecular weight of water vapour/dry air} \\
			/// \lambda &=& 2.45 \frac{MJ}{kg} \mbox{ (FAO 1998,Eq. 8): latent heat of vaporization} \\
			/// R &=& 0.287 \frac{kJ}{kg\ k}\mbox{ Specific gas constant }		\\
			/// \rho_a &=&	\frac{P}{1.01(T+273)R} \mbox{ (FAO 1998,Box. 6): Mean air density at constant pressure} \\
			/// e_s &=& \mbox{ Saturated vapor pressure (see Atmosphere) in } kPa \\
			/// e_a &=& \mbox{ Actual vapor pressure (see Atmosphere) in } kPa \\
			/// r_a &=& \frac{\ln\left(\frac{2-d}{z_{om}}\right)\ln\left(\frac{2-d}{z_{oh}}\right)}{k^2 u_2} \mbox{ (FAO 1998, Eq. 4/Box 4): Aerodynamic resitance in } \frac s m \\
			/// && d=\frac 2 3 h,z_{om}=0.123 h,z_{oh}=0.1 z_{om}, k=0.41 \\
			/// h &=& \mbox{ Vegetation height in }m \\
			/// u_2 &=& \mbox{ Windspeed in 2m above ground (see Atmosphere) } \frac m s \\
			/// r_s &=& \frac{r_l}{LAI_{Active}} \mbox{ (FAO 1998, Eq. 5/Box 5): bulk surface resistance} \frac s m \\
			/// && r_l=100 \frac s m, LAI_{Active}=0.5 LAI
			/// \f}
			class PenmanMonteithET : public cmf::water::flux_connection {

			protected:
				std::tr1::weak_ptr<cmf::upslope::SoilLayer> sw;
				real calc_q(cmf::math::Time t);
				void NewNodes()
				{
					sw=cmf::upslope::SoilLayer::cast(left_node());
				}
			public:

				bool daily;
				PenmanMonteithET(cmf::upslope::SoilLayer::ptr source,cmf::water::flux_node::ptr ET_target) 
					: flux_connection(source,ET_target,"Penman Monteith transpiration"),sw(source) {
						NewNodes();
				}
				static real r_s(const cmf::upslope::vegetation::Vegetation & veg) ;
				static real r_a(cmf::atmosphere::Weather A,real  veg_height) ;
				static void use_for_cell(cmf::upslope::Cell & cell);
			};

			/// @ingroup ET
			/// Calculates the actual transpiration and the soil evaporation from a soil layer
			class ShuttleworthWallaceET : public cmf::water::flux_connection {
			protected:
				std::tr1::weak_ptr<cmf::upslope::SoilLayer> m_SoilLayer;
				std::tr1::weak_ptr<cmf::water::WaterStorage> m_waterstorage;
				cmf::upslope::Cell& m_cell;
				virtual real calc_q(cmf::math::Time t);
				void NewNodes()
				{
					m_SoilLayer=cmf::upslope::SoilLayer::cast(left_node());
					if (!m_SoilLayer.expired())
						m_waterstorage= cmf::water::WaterStorage::cast(left_node());
					else
						m_waterstorage.reset();
				}
			public:
				ShuttleworthWallaceET(cmf::water::WaterStorage::ptr source,cmf::water::flux_node::ptr ET_target,cmf::upslope::Cell& cell,std::string Type="Shuttleworth Wallace get_evaporation") 
					: cmf::water::flux_connection(source,ET_target,Type),m_cell(cell) {
						NewNodes();
				}
				static void use_for_cell(cmf::upslope::Cell& cell);

			};
			/// @ingroup ET
			/// Calculates the Evapotranspiration using Hargreave's equation
			///
			/// @todo document Hargreave
			class HargreaveET : public cmf::water::flux_connection {
			protected:
				std::tr1::weak_ptr<cmf::upslope::SoilLayer> sw;
				real calc_q(cmf::math::Time t);
				void NewNodes()
				{
					sw=cmf::upslope::SoilLayer::cast(left_node());
				}
			public:
				HargreaveET(cmf::upslope::SoilLayer::ptr source,cmf::water::flux_node::ptr ET_target) 
					: flux_connection(source,ET_target,"Hargreave get_evaporation"),sw(source) {
						NewNodes();
				}
				static void use_for_cell(cmf::upslope::Cell & cell);

			};
			/// @ingroup ET
			/// Calculates the evaporation from a canopy storage
			class CanopyStorageEvaporation : public cmf::water::flux_connection {
			protected:
				const cmf::upslope::Cell & m_cell;
				std::tr1::weak_ptr<cmf::water::WaterStorage> c_stor;
				virtual real calc_q(cmf::math::Time t);
				void NewNodes()
				{
					c_stor=cmf::water::WaterStorage::cast(left_node());
				}
			public:
				CanopyStorageEvaporation(cmf::water::WaterStorage::ptr CanopyStorage,cmf::water::flux_node::ptr ET_target,cmf::upslope::Cell & cell)
					: cmf::water::flux_connection(CanopyStorage,ET_target,"Penman Monteith (canopy) get_evaporation"),m_cell(cell) {
						NewNodes();
				}
			};
			/// @ingroup ET
			/// Calculates evaporation from an open water body
			class PenmanEvaporation : public cmf::water::flux_connection
			{
			protected:
				std::tr1::weak_ptr<cmf::river::OpenWaterStorage> m_source;
				std::auto_ptr<cmf::atmosphere::Meteorology> m_meteo;
				virtual real calc_q(cmf::math::Time t);
				void NewNodes()
				{
					m_source=cmf::river::OpenWaterStorage::cast(left_node());
				}
			public:
				PenmanEvaporation(cmf::river::OpenWaterStorage::ptr source,cmf::water::flux_node::ptr Evap_target,const cmf::atmosphere::Meteorology& meteo);
			};
		}
	}
}

#endif // ET_h__