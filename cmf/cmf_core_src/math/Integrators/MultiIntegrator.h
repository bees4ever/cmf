

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
#ifndef MultiIntegrator_h__
#define MultiIntegrator_h__
#include "Integrator.h"
#include <tr1/memory>
namespace cmf {
	namespace math {
		/// The MultiIntegrator is a wrapper for a bunch integrators. The states of the integrators should not have direct connections over integrator boundaries
		class MultiIntegrator : public cmf::math::Integrator
		{
			typedef std::tr1::shared_ptr<cmf::math::Integrator> integ_ptr;
			typedef std::vector<integ_ptr> integ_vector;
			integ_vector m_integrators;
			std::auto_ptr<cmf::math::Integrator> m_template;
		public:
			int Integrate(cmf::math::Time MaxTime,cmf::math::Time TimeStep);
			virtual cmf::math::MultiIntegrator* Copy() const
			{
				return new MultiIntegrator(*m_template,int(m_integrators.size()));
			}
			virtual void Reset()
			{
				for(integ_vector::iterator it = m_integrators.begin(); it != m_integrators.end(); ++it)
				{
				    (**it).ModelTime(m_Time);
				}
			}
			void AddStatesFromOwner(cmf::math::StateVariableOwner& stateOwner)
			{
				throw std::runtime_error("States can only be added to one integrator of the multi integrator. Use add_state_to_integrator");
			}
			void add_states_to_integrator(cmf::math::StateVariableOwner& stateOwner, int integrator_position)
			{
				integ_ptr& integ=m_integrators.at(integrator_position<0 ? m_integrators.size()-integrator_position : integrator_position);
				integ->AddStatesFromOwner(stateOwner);
			}
			MultiIntegrator(const cmf::math::Integrator& template_integrator,int count)
				: Integrator(), m_template(template_integrator.Copy())
			{
				for (int i = 0; i < count ; ++i)
				{
					cmf::math::Integrator* integ=m_template->Copy();
					integ_ptr Integ(integ);
					m_integrators.push_back(Integ);
				}
			}
		};

	}

}

#endif // MultiIntegrator_h__