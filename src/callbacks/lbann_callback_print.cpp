////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2014-2016, Lawrence Livermore National Security, LLC. 
// Produced at the Lawrence Livermore National Laboratory. 
// Written by the LBANN Research Team (B. Van Essen, et al.) listed in
// the CONTRIBUTORS file. <lbann-dev@llnl.gov>
//
// LLNL-CODE-697807.
// All rights reserved.
//
// This file is part of LBANN: Livermore Big Artificial Neural Network
// Toolkit. For details, see http://software.llnl.gov/LBANN or
// https://github.com/LLNL/LBANN. 
//
// Licensed under the Apache License, Version 2.0 (the "Licensee"); you
// may not use this file except in compliance with the License.  You may
// obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied. See the License for the specific language governing
// permissions and limitations under the license.
//
// lbann_callback_print .hpp .cpp - Callback hooks to print information
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include "lbann/callbacks/lbann_callback_print.hpp"

namespace lbann {

void lbann_callback_print::setup(model* m) {
#ifdef LBANN_VERSION
  lbann_comm* comm = m->get_comm();
  if (comm->am_world_master()) {
    std::cout << "Training with LLNL LBANN version " << LBANN_VERSION << endl;
  }
#endif
}

void lbann_callback_print::on_epoch_begin(model* m) {
  lbann_comm* comm = m->get_comm();
  if (comm->am_world_master()) {
    std::cout << "-----------------------------------------------------------" << std::endl;
    std::cout << "[" << m->get_cur_epoch() << "] Epoch" << std::endl;
    std::cout << "-----------------------------------------------------------" << std::endl;
  }
}

void lbann_callback_print::on_epoch_end(model* m) {
  lbann_comm* comm = m->get_comm();
  if (comm->am_model_master()) {
    /// Report the current score for each metric attached to the model
    for (auto&& metric : m->metrics) {
      double train_score = metric->report_metric(execution_mode::training);
      double validate_score = metric->report_metric(execution_mode::validation);
      if (comm->am_world_master()) {
        std::vector<double> train_scores(comm->get_num_models());
        std::vector<double> validate_scores(comm->get_num_models());
        comm->intermodel_gather(train_score, train_scores);
        comm->intermodel_gather(validate_score, validate_scores);

        for (size_t i = 0; i < train_scores.size(); ++i) {
          std::cout << "Model " << i;
          std::cout << " @" << m->get_cur_step() << " steps";
          std::cout << " Training " << _to_string(metric->type) << ": " << train_scores[i] << "%";
          std::cout << " @" << m->get_cur_validation_step() << " validation steps Validation " << _to_string(metric->type) << ": " << validate_scores[i] << _disp_unit(metric->type);
          std::cout << std::endl;
        }
      } else {
        comm->intermodel_gather(train_score, comm->get_intermodel_master());
        comm->intermodel_gather(validate_score, comm->get_intermodel_master());
      }
    }
    for (Layer* layer : m->get_layers()) {
      layer->epoch_print();
    }
  }
}

void lbann_callback_print::on_test_end(model* m) {
  lbann_comm* comm = m->get_comm();
  if (comm->am_model_master()) {
    /// Report the current score for each metric attached to the model
    for (auto&& metric : m->metrics) {
      double test_score = metric->report_metric(execution_mode::testing);
      if (comm->am_world_master()) {
        std::vector<double> test_scores(comm->get_num_models());
        comm->intermodel_gather(test_score, test_scores);
        for (size_t i = 0; i < test_scores.size(); ++i) {
          std::cout << "Model " << i << " @" << m->get_cur_testing_step() << " testing steps external validation " << _to_string(metric->type) << ": ";
          std::cout << test_scores[i] << _disp_unit(metric->type) << std::endl;
        }
      } else {
        comm->intermodel_gather(test_score, comm->get_intermodel_master());
      }
    }
  }
}

}  // namespace lbann
