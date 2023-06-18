This project has been developed on Windows 10, version 21H2, with the CLion compiler, version 2023.1.3.
It does not require and specific tool or libraries to run.

However, to generate the .csv which are used to plot the different figures, the functions:
- stochastic::Algorithm::simulation()
- stochastic::Visualizer::generateNetworkGraph()
both need a path to where their results are to be put.
