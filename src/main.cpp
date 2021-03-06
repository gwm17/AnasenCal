#include <fstream>
#include <string>
#include <iostream>
#include "DataOrganizer.h"
#include "DeadChannelMap.h"
#include "ZeroCalibrator.h"
#include "GainMatcher.h"
#include "MapChecker.h"
#include "EnergyCalibrator.h"
#include "DataCalibrator.h"



int main(int argc, char** argv) 
{
	std::string option = "";
	if(argc == 2)
	{
		option = argv[1];
		if(option == "--help")
		{
			std::cerr<<"Select an option from the following list to run AnasenCal:"<<std::endl;
			std::cerr<<"--organize-data : converts data to the AnasenEvent format specified in DataStructs.h"<<std::endl;
			std::cerr<<"--zero-offset : calibrates the zero offset of each channel using pulser data"<<std::endl;
			std::cerr<<"--gain-match : performs all gain-matching steps in a single-shot (not recommended)"<<std::endl;
			std::cerr<<"--gain-match-backs : performs first step of gain-matching by aligning all back channels within each detector"<<std::endl;
			std::cerr<<"--gain-match-updown : performs second step of gain-matching by aligning the SX3 front upstream and downstream channels"<<std::endl;
			std::cerr<<"--gain-match-frontback : performs last step of gain-matching by aligning front channels to back channels"<<std::endl;
			std::cerr<<"--calibrate-energy : calibrates the energy of each channel using alpha data"<<std::endl;
			std::cerr<<"--apply-calibrations : applies calibrations to a dataset, generating a new calibrated file"<<std::endl;
			std::cerr<<"These are listed in the order that they should be used to completely calibrate the silicon in an ANASEN dataset"<<std::endl;
			std::cerr<<"AnasenCal should be run using the following formula:"<<std::endl;
			std::cerr<<"./bin/anasencal --<option> <input file>"<<std::endl;
			return 0;
		}
	}

	if(argc != 3) {
		std::cerr<<"Incorrect number of arguments, please specify an operation and an input file."<<std::endl;
		std::cerr<<"./bin/anasencal --<option> <input file>"<<std::endl;
		return 1;
	} 
	option = argv[1];
	
	std::ifstream input(argv[2]);
	if(!input.is_open()) 
	{
		std::cerr<<"Unable to open input file, check that it exists!"<<std::endl;
		return 1;
	}

	std::string junk, ecaloutfile, ecaloutrootfile, channelfile, finaldata;
	std::string rawdata, orgainzedata;
	std::string zcaloutfile, zcaloutrootfile;
	std::string alphadata, pulserdata, rundata;
	std::string backgains, updowngains, frontbackgains;
	std::string backgains_plots, updowngains_plots, frontbackgains_plots;
	int runMin, runMax;
	input>>junk>>rawdata;
	input>>junk>>orgainzedata;
	input>>junk>>runMin>>junk>>runMax;
	input>>junk>>pulserdata;
	input>>junk>>alphadata;
	input>>junk>>rundata;
	input>>junk>>zcaloutrootfile;
	input>>junk>>zcaloutfile;
	input>>junk>>backgains_plots;
	input>>junk>>backgains;
	input>>junk>>updowngains_plots;
	input>>junk>>updowngains;
	input>>junk>>frontbackgains_plots;
	input>>junk>>frontbackgains;
	input>>junk>>ecaloutrootfile;
	input>>junk>>ecaloutfile;
	input>>junk>>channelfile;
	input>>junk>>finaldata;
	input.close();

	std::cout<<"--------ANASEN Gain Matching and Calibration--------"<<std::endl;
	std::cout<<"Option passed: "<<option<<std::endl;
	std::cout<<"-------------------Input Data Used------------------"<<std::endl;
	if(option == "--organize-data")
	{
		std::cout<<"Raw datadir: "<<rawdata<<std::endl;
		std::cout<<"Organized datadir: "<<orgainzedata<<std::endl;
		std::cout<<"Run min: "<<runMin<<" Run max: "<<runMax<<std::endl;
		std::cout<<"----------------------------------------------------"<<std::endl;
		std::cout<<"Converting data from raw root format to orgainzed data structures..."<<std::endl;
		std::string raw_file, organized_file;
		DataOrganizer organ(channelfile);
		for(int i=runMin; i<=runMax; i++)
		{
			raw_file = rawdata + "run-" + std::to_string(i) + ".root";
			if(!std::ifstream(raw_file))
				continue;
			organized_file = orgainzedata + "run-" + std::to_string(i) + ".root";
			std::cout<<"Converting file "<<raw_file<<" to file "<<organized_file<<"..."<<std::endl;
			organ.Run(raw_file, organized_file);
		}
	}
	else if(option == "--zero-offset")
	{
		std::cout<<"Pulser data file: "<<pulserdata<<std::endl;
		std::cout<<"Zero-Offset Histogram File: "<<zcaloutrootfile<<std::endl;
		std::cout<<"Zero-Offset Calibration Output File: "<<zcaloutfile<<std::endl;
		std::cout<<"----------------------------------------------------"<<std::endl;
		std::cout<<"Calibrating zero-offset in every channel using pulser data..."<<std::endl;
		ZeroCalibrator zcal(channelfile);
		zcal.Run(pulserdata, zcaloutrootfile, zcaloutfile);
	}
	else if(option == "--zero-dirty")
	{
		std::cout<<"Alpha data file: "<<alphadata<<std::endl;
		std::cout<<"Zero-Offset Histogram File: "<<zcaloutrootfile<<std::endl;
		std::cout<<"Zero-Offset Calibration Output File: "<<zcaloutfile<<std::endl;
		std::cout<<"----------------------------------------------------"<<std::endl;
		std::cout<<"Attempting to recover busted channels in zero offset with alpha data..."<<std::endl;
		ZeroCalibrator zcal(channelfile);
		zcal.RecoverOffsets(alphadata, "/data1/gwm17/7BeNov2021/calibration_plots/dirtyZero.root", zcaloutfile);
	}
	else if(option == "--gain-match")
	{
		GainMatcher matcher(channelfile, zcaloutfile);
		std::cout<<"Alpha data file: "<<alphadata<<std::endl;
		std::cout<<"Run data file: "<<rundata<<std::endl;
		std::cout<<"Back Gain-matching Histogram File: "<<backgains_plots<<std::endl;
		std::cout<<"Back Gain-matching Output File: "<<backgains<<std::endl;
		std::cout<<"SX3 Upstream-Downstream Gain-matching Histogram File: "<<updowngains_plots<<std::endl;
		std::cout<<"SX3 Upstream-Downstream Gain-matching Output File: "<<updowngains<<std::endl;
		std::cout<<"Front-Back Gain-matching Histogram File: "<<frontbackgains_plots<<std::endl;
		std::cout<<"Front-Back Gain-matching Output File: "<<frontbackgains<<std::endl;
		std::cout<<"----------------------------------------------------"<<std::endl;
		std::cout<<"Gain-matching channels..."<<std::endl;
		std::cout<<"Starting by gain-matching all back (SX3 backs & QQQ wedges) channels..."<<std::endl;
		matcher.MatchBacks(alphadata, backgains_plots, backgains, 3, 1);
		std::cout<<"Finished. Now gain-matching SX3 upstream fronts and downstream fronts..."<<std::endl;
		matcher.MatchSX3UpDown(alphadata, updowngains_plots, updowngains, backgains);
		std::cout<<"Finished. Finally, gain-matching all front channels to all back channels..."<<std::endl;
		matcher.MatchFrontBack(rundata, frontbackgains_plots, frontbackgains, backgains, updowngains);
	}
	else if(option == "--gain-match-backs")
	{
		std::cout<<"Alpha data file: "<<alphadata<<std::endl;
		std::cout<<"Back Gain-matching Histogram File: "<<backgains_plots<<std::endl;
		std::cout<<"Back Gain-matching Output File: "<<backgains<<std::endl;
		std::cout<<"----------------------------------------------------"<<std::endl;
		std::cout<<"Gain-matching all back (SX3 backs & QQQ wedges) channels..."<<std::endl;
		GainMatcher matcher(channelfile, zcaloutfile);
		matcher.MatchBacks(alphadata, backgains_plots, backgains, 3, 1);
	}
	else if(option == "--gain-match-updown")
	{
		std::cout<<"Alpha data file: "<<rundata<<std::endl;
		std::cout<<"SX3 Upstream-Downstream Gain-matching Histogram File: "<<updowngains_plots<<std::endl;
		std::cout<<"SX3 Upstream-Downstream Gain-matching Output File: "<<updowngains<<std::endl;
		std::cout<<"----------------------------------------------------"<<std::endl;
		std::cout<<"Gain-matching SX3 upstream fronts and downstream fronts..."<<std::endl;
		GainMatcher matcher(channelfile, zcaloutfile);
		matcher.MatchSX3UpDown(rundata, updowngains_plots, updowngains, backgains);
	}
	else if(option == "--gain-match-frontback")
	{
		std::cout<<"Run data file: "<<rundata<<std::endl;
		std::cout<<"Front-Back Gain-matching Histogram File: "<<frontbackgains_plots<<std::endl;
		std::cout<<"Front-Back Gain-matching Output File: "<<frontbackgains<<std::endl;
		std::cout<<"----------------------------------------------------"<<std::endl;
		std::cout<<"Gain-matching all front channels to all back channels..."<<std::endl;
		GainMatcher matcher(channelfile, zcaloutfile);
		matcher.MatchFrontBack(rundata, frontbackgains_plots, frontbackgains, backgains, updowngains);
	}
	else if(option == "--check-zoffset")
	{
		std::cout<<"Zero-Offset Calibration Output File: "<<zcaloutfile<<std::endl;
		std::cout<<"----------------------------------------------------"<<std::endl;
		std::cout<<"Running check on the given zero-offset map..."<<std::endl;
		MapChecker checker(channelfile);
		checker.CheckZOffset(zcaloutfile);
	}
	else if(option == "--check-backgains")
	{
		std::cout<<"Back Gain-matching Output File: "<<backgains<<std::endl;
		std::cout<<"----------------------------------------------------"<<std::endl;
		std::cout<<"Running check on the given backs gain-matching map..."<<std::endl;
		MapChecker checker(channelfile);
		checker.CheckBackGainMatch(backgains);
	}
	else if(option == "--check-updowngains")
	{
		std::cout<<"SX3 Upstream-Downstream Gain-matching Output File: "<<updowngains<<std::endl;
		std::cout<<"----------------------------------------------------"<<std::endl;
		std::cout<<"Running check on the given SX3 up-down gain-matching map..."<<std::endl;
		MapChecker checker(channelfile);
		checker.CheckUpDownGainMatch(updowngains);
	}
	else if(option == "--check-frontbackgains")
	{
		std::cout<<"Front-Back Gain-matching Output File: "<<frontbackgains<<std::endl;
		std::cout<<"----------------------------------------------------"<<std::endl;
		std::cout<<"Running check on the given front-back gain-matching map..."<<std::endl;
		MapChecker checker(channelfile);
		checker.CheckFrontBackGainMatch(frontbackgains);
	}
	else if(option == "--calibrate-energy")
	{
		std::cout<<"Alpha data file: "<<alphadata<<std::endl;
		std::cout<<"Energy Calibration Histogram File: "<<ecaloutrootfile<<std::endl;
		std::cout<<"Energy Calibration Output File: "<<ecaloutfile<<std::endl;
		std::cout<<"----------------------------------------------------"<<std::endl;
		std::cout<<"Calibrating the energy of the back channels and QQQ rings..."<<std::endl;
		EnergyCalibrator ecal(channelfile, zcaloutfile, backgains, updowngains, frontbackgains);
		ecal.Run(alphadata, ecaloutrootfile, ecaloutfile);
	}
	else if(option == "--apply-calibrations")
	{
		std::cout<<"Zero-Offset Calibration Output File: "<<zcaloutfile<<std::endl;
		std::cout<<"Back Gain-matching Output File: "<<backgains<<std::endl;
		std::cout<<"SX3 Upstream-Downstream Gain-matching Output File: "<<updowngains<<std::endl;
		std::cout<<"Front-Back Gain-matching Output File: "<<frontbackgains<<std::endl;
		std::cout<<"Energy Calibration Output File: "<<ecaloutfile<<std::endl;
		std::cout<<"Run data file: "<<rundata<<std::endl;
		std::cout<<"Calibrated data file: "<<finaldata<<std::endl;
		std::cout<<"----------------------------------------------------"<<std::endl;
		std::cout<<"Applying calibration to the data set "<<rundata<<"..."<<std::endl;
		DataCalibrator dcal(channelfile, zcaloutfile, backgains, updowngains, frontbackgains, ecaloutfile);
		dcal.Run(rundata, finaldata);
	}
	else if(option == "--dead-channels")
	{
		std::cout<<"Zero-Offset Calibration Output File: "<<zcaloutfile<<std::endl;
		std::cout<<"Back Gain-matching Output File: "<<backgains<<std::endl;
		std::cout<<"SX3 Upstream-Downstream Gain-matching Output File: "<<updowngains<<std::endl;
		std::cout<<"Front-Back Gain-matching Output File: "<<frontbackgains<<std::endl;
		std::cout<<"Energy Calibration Output File: "<<ecaloutfile<<std::endl;
		std::cout<<"----------------------------------------------------"<<std::endl;
		std::cout<<"Generating a dead channel map in etc/DeadChannels.txt..."<<std::endl;
		GenerateDeadChannelMap(zcaloutfile, backgains, updowngains, frontbackgains, ecaloutfile, channelfile, "etc/DeadChannels.txt");
	}
	else
	{
		std::cout<<"----------------------------------------------------"<<std::endl;
		std::cerr<<"Unrecognized option passed. Select from the following list to run AnasenCal:"<<std::endl;
		std::cerr<<"--organize-data : converts data to the AnasenEvent format specified in DataStructs.h"<<std::endl;
		std::cerr<<"--zero-offset : calibrates the zero offset of each channel using pulser data"<<std::endl;
		std::cerr<<"--gain-match : performs all gain-matching steps in a single-shot (not recommended)"<<std::endl;
		std::cerr<<"--gain-match-backs : performs first step of gain-matching by aligning all back channels within each detector"<<std::endl;
		std::cerr<<"--gain-match-updown : performs second step of gain-matching by aligning the SX3 front upstream and downstream channels"<<std::endl;
		std::cerr<<"--gain-match-frontback : performs last step of gain-matching by aligning front channels to back channels"<<std::endl;
		std::cerr<<"--calibrate-energy : calibrates the energy of each channel using alpha data"<<std::endl;
		std::cerr<<"--apply-calibrations : applies calibrations to a dataset, generating a new calibrated file"<<std::endl;
		std::cerr<<"These are listed in the order that they should be used to completely calibrate the silicon in an ANASEN dataset"<<std::endl;
		return 1;
	}
	
	std::cout<<"Finished."<<std::endl;
	std::cout<<"---------------------------------------------------"<<std::endl;

	return 0;
}
