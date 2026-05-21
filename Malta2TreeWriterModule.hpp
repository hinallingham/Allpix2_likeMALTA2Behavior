/**
 * @file
 * @brief Definition of ROOT data file writer module
 *
 * @copyright Copyright (c) 2017-2024 CERN and the Allpix Squared authors.
 * This software is distributed under the terms of the MIT License, copied
 * verbatim in the file "LICENSE.md". In applying this license, CERN does not
 * waive the privileges and immunities granted to it by virtue of its status as
 * an Intergovernmental Organization or submit itself to any jurisdiction.
 * SPDX-License-Identifier: MIT
 */

#include <TFile.h>
#include <TTree.h>

#include <atomic>
#include <core/config/Configuration.hpp>
#include <core/geometry/GeometryManager.hpp>
#include <core/messenger/Messenger.hpp>
#include <core/module/Event.hpp>
#include <core/module/Module.hpp>
#include <map>
#include <string>
#include <set>

namespace allpix {
/**
 * @ingroup Modules
 * @brief Module to write object data to ROOT trees following the MALTA2
 * telescope data format
 *
 * Reference:
 * https://gitlab.cern.ch/malta/Malta2/-/blob/master/src/Malta2Tree.cpp?ref_type=heads
 */
class Malta2TreeWriterModule : public allpix::SequentialModule {
 public:
  /**
   * @brief Constructor for this unique module
   * @param config Configuration object for this module as retrieved from the
   * steering file
   * @param messenger Pointer to the messenger object to allow binding to
   * messages on the bus
   * @param geo_mgr Pointer to the geometry manager, containing the detectors
   */
  Malta2TreeWriterModule(Configuration& config, Messenger* messenger,
                         GeometryManager* geo_mgr);
  /**
   * @brief Destructor deletes the internal objects used to build the ROOT Tree
   */
  ~Malta2TreeWriterModule() override;

  /**
   * @brief Opens the file to write the objects to
   */
  void initialize() override;

  /**
   * @brief Writes the objects fetched to their specific tree, constructing
   * trees on the fly for new objects.
   */
  void run(allpix::Event* event) override;

  /**
   * @brief Add the main configuration and the detector setup to the data file
   * and write it, also write statistics information.
   */
  void finalize() override;

 private:
  allpix::Messenger* messenger_;
  allpix::GeometryManager* geo_mgr_;

  // Object names to include or exclude from writing
  std::set<std::string> include_;
  std::set<std::string> exclude_;

  std::map<std::string, std::unique_ptr<TFile>> output_files_;
  std::map<std::string, std::unique_ptr<TTree>> trees_;

  int run_number_{1};

  // Current event
  uint64_t current_event_{0};

  // Current random seed
  uint64_t current_seed_{0};

  // Branch variables
  uint32_t pixel_;
  uint32_t group_;
  uint32_t parity_;
  uint32_t delay_;
  uint32_t dcolumn_;
  uint32_t chipbcid_;
  uint32_t chipid_;
  uint32_t phase_;
  uint32_t winid_;
  uint32_t bcid_;
  uint32_t l1id_;
  uint32_t run_;
  uint32_t l1idC_;
  float timer_;
  uint32_t isDuplicate_;
  uint32_t idb_;
  uint32_t ithr_;
  uint32_t vlow_;
  uint32_t vhigh_;
};
}  // namespace allpix