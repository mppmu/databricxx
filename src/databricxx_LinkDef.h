#ifndef databricxx_LinkDef_h
#define databricxx_LinkDef_h

#ifdef __CINT__

// Bric.h
#pragma link C++ class dbrx::Bric-;
#pragma link C++ class dbrx::Bric::Terminal-;
#pragma link C++ class dbrx::Bric::OutputTerminal-;
#pragma link C++ class dbrx::Bric::InputTerminal-;
#pragma link C++ class dbrx::BricImpl-;
#pragma link C++ class dbrx::BricWithOutputs-;
#pragma link C++ class dbrx::BricWithInputs-;
#pragma link C++ class dbrx::BricWithInOut-;
#pragma link C++ class dbrx::ImportBric-;
#pragma link C++ class dbrx::ExportBric-;
#pragma link C++ class dbrx::MapperBric-;
#pragma link C++ class dbrx::TransformBric-;
#pragma link C++ class dbrx::ReducerBric-;

// DbrxTools.h
#pragma link C++ class dbrx::DbrxTools-;

// Name.h, NameTable.h
#pragma link C++ class dbrx::Name-;
#pragma link C++ class dbrx::HasName-;
#pragma link C++ class dbrx::HasNameImpl-;

#pragma link C++ class dbrx::NameTable-;

// Props.h
#pragma link C++ typedef dbrx::Props;
#pragma link C++ class dbrx::PropVal-;

// RootIO.h
#pragma link C++ class dbrx::RootIO-;

// RootReflection.h
#pragma link C++ class dbrx::RootReflection-;

// Value.h, HasValue.h
#pragma link C++ class dbrx::Value-;
#pragma link C++ class dbrx::WritableValue-;
#pragma link C++ class dbrx::PrimaryValue-;
#pragma link C++ class dbrx::ValueRef-;
#pragma link C++ class dbrx::ConstValueRef-;

#pragma link C++ class dbrx::HasValue-;
#pragma link C++ class dbrx::HasWritableValue-;
#pragma link C++ class dbrx::HasPrimaryValue-;
#pragma link C++ class dbrx::HasValueRef-;
#pragma link C++ class dbrx::HasConstValueRef-;

// brics.h

#endif // __CINT__

#endif // databricxx_LinkDef_h
