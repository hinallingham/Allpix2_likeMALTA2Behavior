/**
 * @file
 * @brief Implementation of ROOT data file writer module (Multi-Detector Version)
 */

#include "Malta2TreeWriterModule.hpp"

#include <TBranchElement.h>
#include <TClass.h>
#include <TProcessID.h>
#include <core/utils/log.h>
#include <core/utils/type.h>
#include <objects/objects.h>
#include <tools/ROOT.h>

#include <core/config/ConfigReader.hpp>
#include <fstream>
#include <iomanip>
#include <objects/Object.hpp>
#include <string>
#include <utility>

using namespace allpix;

Malta2TreeWriterModule::Malta2TreeWriterModule(Configuration& config,
                                               Messenger* messenger,
                                               GeometryManager* geo_mgr)
    : SequentialModule(config), messenger_(messenger), geo_mgr_(geo_mgr) {
  allow_multithreading();

  messenger_->bindMulti<PixelHitMessage>(this, MsgFlags::IGNORE_NAME);
  
  run_number_ = config_.get<int>("run_number", 1);
}

Malta2TreeWriterModule::~Malta2TreeWriterModule() {}

void Malta2TreeWriterModule::initialize() {
  
  auto detectors = geo_mgr_->getDetectors();

  int plane_index = 0;

  for(const auto& detector : detectors) {
      std::string det_name = detector->getName();
      
      std::ostringstream ss;
      ss << "run_" << std::setw(6) << std::setfill('0') << run_number_ << "_" << plane_index << ".root";
      
      std::string file_name = createOutputFile(ss.str(), "root", true);
      auto output_file = std::make_unique<TFile>(file_name.c_str(), "RECREATE");
      output_file->cd();

      auto tree = std::make_unique<TTree>("MALTA", ("MALTA2 Data for " + det_name).c_str());

      tree->Branch("pixel", &pixel_, "pixel/i");
      tree->Branch("group", &group_, "group/i");
      tree->Branch("parity", &parity_, "parity/i");
      tree->Branch("delay", &delay_, "delay/i");
      tree->Branch("dcolumn", &dcolumn_, "dcolumn/i");
      tree->Branch("chipbcid", &chipbcid_, "chipbcid/i");
      tree->Branch("chipid", &chipid_, "chipid/i");
      tree->Branch("phase", &phase_, "phase/i");
      tree->Branch("winid", &winid_, "winid/i");
      tree->Branch("bcid", &bcid_, "bcid/i");
      tree->Branch("runNumber", &run_, "runNumber/i");
      tree->Branch("l1id", &l1id_, "l1id/i");
      tree->Branch("l1idC", &l1idC_, "l1idC/i");
      tree->Branch("isDuplicate", &isDuplicate_, "isDuplicate/i");
      tree->Branch("timer", &timer_, "timer/f");

      // マップに格納して保持
      output_files_[det_name] = std::move(output_file);
      trees_[det_name] = std::move(tree);
      
      LOG(STATUS) << "Initialized output file for detector [" << det_name << "] -> " << file_name;
      plane_index++;
  }
}

void Malta2TreeWriterModule::run(Event* event) {
    auto root_lock = root_process_lock();

    std::vector<std::shared_ptr<PixelHitMessage>> hit_messages;

    try {
        hit_messages = messenger_->fetchMultiMessage<PixelHitMessage>(this, event);
    } catch(const MessageNotFoundException& e) {
        return;
    }

    for(auto& msg : hit_messages) {

        std::string det_name = msg->getDetector()->getName();
        
        if(trees_.find(det_name) == trees_.end()) {
            continue; 
        }

        for(auto& hit : msg->getData()) {
            
            unsigned int x = hit.getPixel().getIndex().x();
            unsigned int y = hit.getPixel().getIndex().y();
            
            dcolumn_ = x / 2;
            group_   = y / 16;
            parity_  = ((y % 16) >= 8) ? 1 : 0;
            
            unsigned int pix_base   = (x % 2 != 0) ? 8 : 0;
            unsigned int pix_offset = y % 8;
            unsigned int pix_index  = pix_base + pix_offset;
            pixel_ = (1 << pix_index); 

            double time_ns = hit.getLocalTime(); 
            bcid_  = static_cast<unsigned int>(time_ns / 25.0);
            
            double remainder_25 = fmod(time_ns, 25.0);
            winid_ = static_cast<unsigned int>(remainder_25 / 3.125);
            
            double remainder_3 = fmod(remainder_25, 3.125);
            phase_ = static_cast<unsigned int>(remainder_3 / 0.39);

            l1id_ = static_cast<unsigned int>(event->number); 
            run_  = static_cast<unsigned int>(run_number_);              
            isDuplicate_ = 0;       
            
            trees_[det_name]->Fill();
        }
    }
}

void Malta2TreeWriterModule::finalize() {
  LOG(STATUS) << "Writing configuration metadata and closing files...";

  ConfigManager* conf_manager = getConfigManager();

  for (auto& pair : output_files_) {
      std::string det_name = pair.first;
      TFile* file = pair.second.get();
      file->cd();

      TDirectory* config_dir = file->mkdir("config");
      config_dir->cd();

      auto* global_dir = config_dir->mkdir("Allpix");
      for (auto& key_value : conf_manager->getGlobalConfiguration().getAll()) {
        global_dir->WriteObject(&key_value.second, key_value.first.c_str());
      }

      for (const auto& config : conf_manager->getInstanceConfigurations()) {
        auto unique_name = config.getName();
        auto identifier = config.get<std::string>("identifier");
        if (!identifier.empty()) {
          unique_name += ":";
          unique_name += identifier;
        }
        auto* section_dir = config_dir->mkdir(unique_name.c_str());
        for (auto& key_value : config.getAll()) {
          if (key_value.first == "identifier") continue;
          section_dir->WriteObject(&key_value.second, key_value.first.c_str());
        }
      }

      file->Write();
  }
}