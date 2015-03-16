{

// Use any class in databricxx .rootmap file to make ROOT/Cling load the
// databricxx library:

dbrx::PropVal();


// Define a nested/hierarchial parameter structure:


struct MyParams: public dbrx::ParamGroup {
	Param<bool> someBool{this, "someBool", "Param description", true};
	Param<double> someDouble{this, "someDouble", "Some double param", 0.1};
	Param<std::string> someString{this, "someString", "", "default value"};
	Param<std::vector<int>> someVector{this, "someVector", "", {1, 2 , 3, 4}};

	struct : public dbrx::ParamGroup {
		Param<int32_t> small{this, "small", "Some 32-bit int param", 4};
		Param<int64_t> big{this, "big", "Some 64-bit int param", 17179869184};

		using ParamGroup::ParamGroup;
	} integers{this, "integers"};
};


// Parameter group instance:

MyParams myParams1;


// Change default values:

myParams1.integers.small = 35;
myParams1.someVector = {6, 7, 8};
myParams1.integers.small += myParams1.someVector.get()[1];
myParams1.someDouble *= myParams1.integers.small;


// Write parameters to file:

cout << "myParams1 = " << myParams1.getConfig() << endl;
myParams1.getConfig().toFile("out-config.json");


// Read parameters from file:

MyParams myParams2;
myParams2.applyConfig(dbrx::PropVal::fromFile("out-config.json"));
cout << "myParams2 = " << myParams2.getConfig() << endl;


// Compare:

assert (myParams1.getConfig() == myParams2.getConfig());

}
