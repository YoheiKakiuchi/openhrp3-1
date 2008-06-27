// -*- mode: c++; indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*-
/*!
 * @file  SamplePD.cpp
 * @brief Sample PD component
 * $Date$
 *
 * $Id$
 */

#include "SamplePD.h"

#include <iostream>

#define DOF (29)
#define TIMESTEP 0.002

#define ANGLE_FILE "etc/angle.dat"
#define VEL_FILE   "etc/vel.dat"
#define ACC_FILE   "etc/acc.dat"

#define GAIN_FILE  "etc/PDgain.dat"

namespace {
  const bool CONTROLLER_BRIDGE_DEBUG = false;
}


// Module specification
// <rtc-template block="module_spec">
static const char* samplepd_spec[] =
  {
    "implementation_id", "SamplePD",
    "type_name",         "SamplePD",
    "description",       "Sample PD component",
    "version",           "0.1",
    "vendor",            "AIST",
    "category",          "Generic",
    "activity_type",     "DataFlowComponent",
    "max_instance",      "10",
    "language",          "C++",
    "lang_type",         "compile",
    // Configuration variables

    ""
  };
// </rtc-template>

SamplePD::SamplePD(RTC::Manager* manager)
  : RTC::DataFlowComponentBase(manager),
    // <rtc-template block="initializer">
    m_angleIn("angle", m_angle),
    m_torqueOut("torque", m_torque),
    
    // </rtc-template>
    dummy(0),
    qold(DOF)
{
  if( CONTROLLER_BRIDGE_DEBUG )
  {
    std::cout << "SamplePD::SamplePD" << std::endl;
  }

  // Registration: InPort/OutPort/Service
  // <rtc-template block="registration">
  // Set InPort buffers
  registerInPort("angle", m_angleIn);
  
  // Set OutPort buffer
  registerOutPort("torque", m_torqueOut);
  
  // Set service provider to Ports
  
  // Set service consumers to Ports
  
  // Set CORBA Service Ports
  
  // </rtc-template>

  Pgain = new double[DOF];
  Dgain = new double[DOF];

  if (access(ANGLE_FILE, 0)){
    std::cerr << ANGLE_FILE << " not found" << std::endl;
  }else{
    angle.open(ANGLE_FILE);
  }

  if (access(VEL_FILE, 0)){
    std::cerr << VEL_FILE << " not found" << std::endl;
  }else{
    vel.open(VEL_FILE);
  }

  if (access(GAIN_FILE, 0)){
    std::cerr << GAIN_FILE << " not found" << std::endl;
  }else{
    gain.open(GAIN_FILE);
    for (int i=0; i<DOF; i++){
      gain >> Pgain[i];
      gain >> Dgain[i];
    }
    gain.close();
  }
  m_torque.data.length(DOF);
  m_angle.data.length(DOF);

}

SamplePD::~SamplePD()
{
  if (angle.is_open()) angle.close();
  if (vel.is_open()) vel.close();
  delete [] Pgain;
  delete [] Dgain;
}


RTC::ReturnCode_t SamplePD::onInitialize()
{
  // <rtc-template block="bind_config">
  // Bind variables and configuration variable
  if( CONTROLLER_BRIDGE_DEBUG )
  {
    std::cout << "onInitialize" << std::endl;
  }
  // </rtc-template>
  return RTC::RTC_OK;
}



/*
RTC::ReturnCode_t SamplePD::onFinalize()
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t SamplePD::onStartup(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t SamplePD::onShutdown(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

RTC::ReturnCode_t SamplePD::onActivated(RTC::UniqueId ec_id)
{
	std::cout << "on Activated" << std::endl;
	angle.seekg(0);
	vel.seekg(0);
	
	m_angleIn.update();
	
	for(int i=0; i < DOF; ++i){
		qold[i] = m_angle.data[i];
	}
	
	return RTC::RTC_OK;
}

/*
RTC::ReturnCode_t SamplePD::onDeactivated(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/


RTC::ReturnCode_t SamplePD::onExecute(RTC::UniqueId ec_id)
{
  if( CONTROLLER_BRIDGE_DEBUG )
  {
    std::cout << "onExecute" << std::endl;
		std::string	localStr;
		std::cin >> localStr; 
  }

  m_angleIn.update();

  double q_ref, dq_ref;
  angle >> q_ref; vel >> dq_ref;// skip time
  for (int i=0; i<DOF; i++){
    angle >> q_ref;
    vel >> dq_ref;
    double q = m_angle.data[i];
    double dq = (q - qold[i]) / TIMESTEP;
    qold[i] = q;
    
    m_torque.data[i] = -(q - q_ref) * Pgain[i] - (dq - dq_ref) * Dgain[i];
  }
  
  m_torqueOut.write();
  
  return RTC::RTC_OK;
}


/*
  RTC::ReturnCode_t SamplePD::onAborting(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t SamplePD::onError(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t SamplePD::onReset(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t SamplePD::onStateUpdate(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t SamplePD::onRateChanged(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/



extern "C"
{

	DllExport void SamplePDInit(RTC::Manager* manager)
	{
		RTC::Properties profile(samplepd_spec);
		manager->registerFactory(profile,
								 RTC::Create<SamplePD>,
								 RTC::Delete<SamplePD>);
	}

};
