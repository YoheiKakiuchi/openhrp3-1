/**
   \file
   \author Shin'ichiro Nakaoka
*/

#ifndef OPENHRP_CONTROLLER_BRIDGE_VIRTUAL_ROBOT_PORT_HANDLER_H_INCLUDED
#define OPENHRP_CONTROLLER_BRIDGE_VIRTUAL_ROBOT_PORT_HANDLER_H_INCLUDED

#include <boost/shared_ptr.hpp>

#include <rtm/RTC.h>
#include <rtm/PortBase.h>
#include <rtm/OutPort.h>
#include <rtm/InPort.h>
#include <rtm/idl/BasicDataTypeStub.h>

#include <DynamicsSimulator.h>

#include "BridgeConf.h"


namespace OpenHRP {

  namespace ControllerBridge {

    class Controller_impl;

    class PortHandler
    {
    public:
      virtual ~PortHandler();
      RTC::Port_var portRef;
    };

    typedef boost::shared_ptr<PortHandler> PortHandlerPtr;


    class OutPortHandler : public PortHandler
    {
    public:
      virtual void inputDataFromSimulator(Controller_impl* controller) = 0;
      virtual void writeDataToPort() = 0;
    };

    typedef boost::shared_ptr<OutPortHandler> OutPortHandlerPtr;


    class InPortHandler : public PortHandler
    {
    public:
      virtual void outputDataToSimulator(Controller_impl* controller) = 0;
      virtual void readDataFromPort(Controller_impl* controller) = 0;
    };

    typedef boost::shared_ptr<InPortHandler> InPortHandlerPtr;


    class SensorStateOutPortHandler : public OutPortHandler
    {
    public:
      SensorStateOutPortHandler(PortInfo& info);
      virtual void inputDataFromSimulator(Controller_impl* controller);
      virtual void writeDataToPort();
      RTC::OutPort<RTC::TimedDoubleSeq> outPort;
    private:
      RTC::TimedDoubleSeq values;
      DataTypeId dataTypeId;
    };


    class LinkDataOutPortHandler : public OutPortHandler
    {
    public:
      LinkDataOutPortHandler(PortInfo& info);
      virtual void inputDataFromSimulator(Controller_impl* controller);
      virtual void writeDataToPort();
      RTC::OutPort<RTC::TimedDoubleSeq> outPort;
    private:
      std::string linkName;
      DynamicsSimulator::LinkDataType linkDataType;
      RTC::TimedDoubleSeq value;
    };


    class SensorDataOutPortHandler : public OutPortHandler
    {
    public:
      SensorDataOutPortHandler(PortInfo& info);
      virtual void inputDataFromSimulator(Controller_impl* controller);
      virtual void writeDataToPort();
      RTC::OutPort<RTC::TimedDoubleSeq> outPort;
    private:
      RTC::TimedDoubleSeq value;
      std::string sensorName;
    };


    class ColorImageOutPortHandler : public OutPortHandler
    {
    public:
      ColorImageOutPortHandler(PortInfo& info);
      virtual void inputDataFromSimulator(Controller_impl* controller);
      virtual void writeDataToPort();
      RTC::OutPort<RTC::TimedLongSeq> outPort;
    private:
      RTC::TimedLongSeq image;
      int cameraId;
    };


    class GrayScaleImageOutPortHandler : public OutPortHandler
    {
    public:
      GrayScaleImageOutPortHandler(PortInfo& info);
      virtual void inputDataFromSimulator(Controller_impl* controller);
      virtual void writeDataToPort();
      RTC::OutPort<RTC::TimedOctetSeq> outPort;
    private:
      RTC::TimedOctetSeq image;
      int cameraId;
    };


    class DepthImageOutPortHandler : public OutPortHandler
    {
    public:
      DepthImageOutPortHandler(PortInfo& info);
      virtual void inputDataFromSimulator(Controller_impl* controller);
      virtual void writeDataToPort();
      RTC::OutPort<RTC::TimedFloatSeq> outPort;
    private:
      RTC::TimedFloatSeq image;
      int cameraId;
    };
    
  
    class JointDataSeqInPortHandler : public InPortHandler
    {
    public:
      JointDataSeqInPortHandler(PortInfo& info);
      virtual void outputDataToSimulator(Controller_impl* controller);
      virtual void readDataFromPort(Controller_impl* controller);
      RTC::InPort<RTC::TimedDoubleSeq> inPort;
    private:
      RTC::TimedDoubleSeq values;
      DynamicsSimulator::LinkDataType linkDataType;
    };

  }

}
    
#endif