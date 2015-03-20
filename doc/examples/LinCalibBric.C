class LinCalibBric: public dbrx::TransformBric {
public:
	Input<double> input{this};

	Output<double> output{this};

	Param<double> offset{this, "offset", "Offset", 0.0};
	Param<double> slope{this, "slope", "Slope", 1.0};


	void processInput() override {
		output = offset + slope * input;
	}

	using dbrx::TransformBric::TransformBric;
};
