#ifndef databricxx_LinkDef_h
#define databricxx_LinkDef_h

#ifdef __CINT__

// Bric.h
#pragma link C++ class dbrx::Bric-;
#pragma link C++ class dbrx::BricWithOutputs-;
#pragma link C++ class dbrx::BricWithInputs-;
#pragma link C++ class dbrx::BricWithInOut-;
#pragma link C++ class dbrx::InputBric-;
#pragma link C++ class dbrx::OutputBric-;
#pragma link C++ class dbrx::FilterBric-;
#pragma link C++ class dbrx::MapperBric-;
#pragma link C++ class dbrx::ReducerBric-;

// DbrxTools.h
#pragma link C++ class dbrx::DbrxTools-;

// Name.h
#pragma link C++ class dbrx::Name-;
#pragma link C++ class dbrx::Named-;
#pragma link C++ class dbrx::NamedImpl-;

// Prop.h
#pragma link C++ class dbrx::Prop-;

// Reflection.h
#pragma link C++ class dbrx::Reflection-;

// RootIO.h
#pragma link C++ class dbrx::RootIO-;

// Value.h
#pragma link C++ class dbrx::Value-;

// brics.h

#endif // __CINT__

#endif // databricxx_LinkDef_h
