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
// lbann_callback_io .hpp .cpp - Callback hooks for learning rate schedules
////////////////////////////////////////////////////////////////////////////////

#ifndef LBANN_CALLBACKS_IO_HPP_INCLUDED
#define LBANN_CALLBACKS_IO_HPP_INCLUDED

#include <unordered_set>
#include <unordered_map>
#include "lbann/callbacks/lbann_callback.hpp"

namespace lbann {

// Different schedules should inherit from lbann_callback_io.

/**
 * Base class for learning rate schedules.
 * Child classes should implement the schedule method to make changes.
 */
class lbann_callback_io : public lbann_callback {
public:
  lbann_callback_io();
  /** Only apply to specific layers. */
  lbann_callback_io(std::unordered_set<uint> _layers);
  // /** Do some initialization. */
  // void on_train_begin(model* m);
  // /** Apply the learning rate schedule. */
  // void on_epoch_begin(model* m);
  /** Report how much I/O has occured per data reader */
  void on_epoch_end(model* m);
  void on_test_end(model* m);
private:
  /** Indicies of layers to monitor. */
  std::unordered_set<uint> layer_indices;
};

}  // namespace lbann

#endif  // LBANN_CALLBACKS_IO_HPP_INCLUDED
