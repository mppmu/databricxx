#ifndef databricxx_LinkDef_h
#define databricxx_LinkDef_h

#ifdef __CINT__

// basicbrics.h

// funcbrics.h

// collbrics.h

// propsbrics.h
#pragma link C++ class dbrx::JSON2PropVal-;
#pragma link C++ class dbrx::PropVal2JSON-;
#pragma link C++ class dbrx::PropsBuilder-;
#pragma link C++ class dbrx::PropsSplitter-;

// rootiobrics.h
#pragma link C++ class dbrx::RootTreeReader-;
#pragma link C++ class dbrx::RootTreeWriter-;
#pragma link C++ class dbrx::RootFileReader-;
#pragma link C++ class dbrx::RootFileWriter-;

// textbrics.h
#pragma link C++ class dbrx::TextFileReader-;
#pragma link C++ class dbrx::TextFileWriter-;

// ApplicationBric.h
#pragma link C++ class dbrx::ApplicationBric-;

// ApplicationConfig.h
#pragma link C++ class dbrx::ApplicationConfig-;

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

// ManagedStream.h
#pragma link C++ class dbrx::ManagedStream-;
#pragma link C++ class dbrx::ManagedInputStream-;
#pragma link C++ class dbrx::ManagedOutputStream-;

// MRBric.h
#pragma link C++ class dbrx::MRBric-;

// Name.h, NameTable.h
#pragma link C++ class dbrx::Name-;
#pragma link C++ class dbrx::HasName-;
#pragma link C++ class dbrx::HasNameImpl-;

#pragma link C++ class dbrx::NameTable-;

// Props.h
#pragma link C++ class dbrx::PropVal-;

// Printable.h
#pragma link C++ class dbrx::Printable-;

// RootCollection.h

// RootHistBuilder.h

// RootIO.h
#pragma link C++ class dbrx::RootIO-;

// RootRndGen.h
#pragma link C++ class dbrx::RootRndGen-;

// TypeReflection.h
#pragma link C++ class dbrx::TypeReflection-;

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

// WrappedTObj.h
#pragma link C++ class dbrx::AbstractWrappedTObj-;

#endif // __CINT__

#endif // databricxx_LinkDef_h
