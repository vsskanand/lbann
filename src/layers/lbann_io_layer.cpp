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
////////////////////////////////////////////////////////////////////////////////

#include "lbann/layers/lbann_io_layer.hpp"
#include "lbann/utils/lbann_exception.hpp"
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;
using namespace El;

lbann::io_layer::io_layer(lbann_comm* comm, uint mini_batch_size, std::map<execution_mode, DataReader*> data_readers, std::vector<regularizer*> regs, bool data_sets_span_models, bool for_regression)
  : Layer(0, comm, NULL, mini_batch_size, activation_type::ID, regs), 
    m_training_dataset(data_readers[execution_mode::training]),  m_testing_dataset(data_readers[execution_mode::testing]), m_validation_dataset(data_readers[execution_mode::validation]), m_data_sets_span_models(data_sets_span_models), m_for_regression(for_regression)
{
  if(m_training_dataset.data_reader != NULL) {
    m_training_dataset.total_samples = m_training_dataset.data_reader->getNumData();
  }

  if(m_validation_dataset.data_reader != NULL) {
    m_validation_dataset.total_samples = m_validation_dataset.data_reader->getNumData();
  }

  if(m_testing_dataset.data_reader != NULL) {
    m_testing_dataset.total_samples = m_testing_dataset.data_reader->getNumData();
  }
}

// lbann::io_layer::io_layer(lbann_comm* comm, uint mini_batch_size, DataReader *training_data_reader)
//   : io_layer(comm, mini_batch_size, training_data_reader, NULL, {}) {}

lbann::DataReader *lbann::io_layer::set_training_data_reader(DataReader *data_reader) {
  /// @todo put in a check to make sure that this is a data reader
  /// that matches what was already there
  DataReader *old_data_reader = m_training_dataset.data_reader;
  m_training_dataset.data_reader = data_reader;
  m_training_dataset.num_samples_processed = 0;
  m_training_dataset.total_samples = data_reader->getNumData();
  return old_data_reader;
}

lbann::DataReader *lbann::io_layer::set_validation_data_reader(DataReader *data_reader) {
  /// @todo put in a check to make sure that this is a data reader
  /// that matches what was already there
  DataReader *old_data_reader = m_validation_dataset.data_reader;
  m_validation_dataset.data_reader = data_reader;
  m_validation_dataset.num_samples_processed = 0;
  m_validation_dataset.total_samples = data_reader->getNumData();
  return old_data_reader;
}

lbann::DataReader *lbann::io_layer::set_testing_data_reader(DataReader *data_reader) {
  /// @todo put in a check to make sure that this is a data reader
  /// that matches what was already there
  DataReader *old_data_reader = m_testing_dataset.data_reader;
  m_testing_dataset.data_reader = data_reader;
  m_testing_dataset.num_samples_processed = 0;
  m_testing_dataset.total_samples = data_reader->getNumData();
  return old_data_reader;
}

lbann::DataReader *lbann::io_layer::select_data_reader() {
  switch(m_execution_mode) {
  case execution_mode::training:
    return m_training_dataset.data_reader;
    break;
  case execution_mode::validation:
    return m_validation_dataset.data_reader;
    break;
  case execution_mode::testing:
    return m_testing_dataset.data_reader;
    break;
  // case prediction:
  //   return m_prediction_data_reader;
  //   break;
  default:
    throw -1;
  }
}

long lbann::io_layer::update_num_samples_processed(long num_samples) {
  switch(m_execution_mode) {
  case execution_mode::training:
    m_training_dataset.num_samples_processed += num_samples;
    return m_training_dataset.num_samples_processed;
    break;
  case execution_mode::validation:
    m_validation_dataset.num_samples_processed += num_samples;
    return m_validation_dataset.num_samples_processed;
    break;
  case execution_mode::testing:
    m_testing_dataset.num_samples_processed += num_samples;
    return m_testing_dataset.num_samples_processed;
    break;
  // case prediction:
  //   return m_prediction_data_reader;
  //   break;
  default:
    throw lbann_exception("lbann_io_layer: invalid execution phase");
  }
}


long lbann::io_layer::get_linearized_data_size() {
  long linearized_data_size = -1;

  /// @todo NumNeurons should be hidden inside of an accessor function

  if(m_training_dataset.data_reader != NULL) {
    long tmp_linearized_data_size = m_training_dataset.data_reader->get_linearized_data_size();
    if(linearized_data_size != -1 && linearized_data_size != tmp_linearized_data_size) {
      throw lbann_exception("lbann_io_layer: training data set size does not match the currently established data set size");
    }
    linearized_data_size = tmp_linearized_data_size;
  }

  if(m_validation_dataset.data_reader != NULL) {
    long tmp_linearized_data_size = m_validation_dataset.data_reader->get_linearized_data_size();
    if(linearized_data_size != -1 && linearized_data_size != tmp_linearized_data_size) {
      throw lbann_exception("lbann_io_layer: validation data set size does not match the currently established data set size");
    }
    linearized_data_size = tmp_linearized_data_size;
  }

  if(m_testing_dataset.data_reader != NULL) {
    long tmp_linearized_data_size = m_testing_dataset.data_reader->get_linearized_data_size();
    if(linearized_data_size != -1 && linearized_data_size != tmp_linearized_data_size) {
      throw lbann_exception("lbann_io_layer: testing data set size does not match the currently established data set size");
    }
    linearized_data_size = tmp_linearized_data_size;
  }

  return linearized_data_size;
}

long lbann::io_layer::get_linearized_label_size() {
  if (is_for_regression()) return static_cast<long>(1);

  long linearized_label_size = -1;

  /// @todo NumNeurons should be hidden inside of an accessor function

  if(m_training_dataset.data_reader != NULL) {
    long tmp_linearized_label_size = m_training_dataset.data_reader->get_linearized_label_size();
    if(linearized_label_size != -1 && linearized_label_size != tmp_linearized_label_size) {
      throw lbann_exception("lbann_io_layer: training label set size does not match the currently established label set size");
    }
    linearized_label_size = tmp_linearized_label_size;
  }

  if(m_validation_dataset.data_reader != NULL) {
    long tmp_linearized_label_size = m_validation_dataset.data_reader->get_linearized_label_size();
    if(linearized_label_size != -1 && linearized_label_size != tmp_linearized_label_size) {
      throw lbann_exception("lbann_io_layer: validation label set size does not match the currently established label set size");
    }
    linearized_label_size = tmp_linearized_label_size;
  }

  if(m_testing_dataset.data_reader != NULL) {
    long tmp_linearized_label_size = m_testing_dataset.data_reader->get_linearized_label_size();
    if(linearized_label_size != -1 && linearized_label_size != tmp_linearized_label_size) {
      throw lbann_exception("lbann_io_layer: testing label set size does not match the currently established label set size");
    }
    linearized_label_size = tmp_linearized_label_size;
  }

  return linearized_label_size;
}

void lbann::io_layer::setup_data_readers_for_training(int base_offset, int stride, int model_offset) {
  if(m_training_dataset.data_reader != NULL) {
    m_training_dataset.data_reader->setup(base_offset, stride, model_offset, comm);
  }
}

/** Do not spread data readers that are used for evaluation across multiple models.  
 * Allow each model instance to use the full data set for evaluation so that each model is fairly compared.
 */
void lbann::io_layer::setup_data_readers_for_evaluation(int base_offset, int stride, int model_offset) {
  if(m_validation_dataset.data_reader != NULL) {
    m_validation_dataset.data_reader->setup(base_offset, stride, model_offset, NULL/*comm*/);
  }

  if(m_testing_dataset.data_reader != NULL) {
    m_testing_dataset.data_reader->setup(base_offset, stride, model_offset, NULL/*comm*/);
  }
  return;
}

bool lbann::io_layer::saveToCheckpointShared(persist& p)
{
    // rank 0 writes the file
    if (p.m_rank == 0) {
        p.write_uint64(persist_type::train, "reader_train_processed",
            (uint64_t) m_training_dataset.num_samples_processed);
        p.write_uint64(persist_type::train, "reader_train_total",
            (uint64_t) m_training_dataset.total_samples);

        p.write_uint64(persist_type::train, "reader_test_processed",
            (uint64_t) m_testing_dataset.num_samples_processed);
        p.write_uint64(persist_type::train, "reader_test_total",
            (uint64_t) m_testing_dataset.total_samples);

        p.write_uint64(persist_type::train, "reader_validate_processed",
            (uint64_t) m_validation_dataset.num_samples_processed);
        p.write_uint64(persist_type::train, "reader_validate_total",
            (uint64_t) m_validation_dataset.total_samples);
    }

    return true;
}

struct dataset_header {
    uint64_t train_proc;
    uint64_t train_total;
    uint64_t test_proc;
    uint64_t test_total;
    uint64_t validate_proc;
    uint64_t validate_total;
};

bool lbann::io_layer::loadFromCheckpointShared(persist& p)
{
    // rank 0 reads the file
    dataset_header header;
    if (p.m_rank == 0) {
        p.read_uint64(persist_type::train, "reader_train_processed",    &header.train_proc);
        p.read_uint64(persist_type::train, "reader_train_total",        &header.train_total);
        p.read_uint64(persist_type::train, "reader_test_processed",     &header.test_proc);
        p.read_uint64(persist_type::train, "reader_test_total",         &header.test_total);
        p.read_uint64(persist_type::train, "reader_validate_processed", &header.validate_proc);
        p.read_uint64(persist_type::train, "reader_validate_total",     &header.validate_total);
    }

    // TODO: assumes homogeneous hardware
    // broadcast data from rank 0
    MPI_Bcast(&header, sizeof(header), MPI_BYTE, 0, MPI_COMM_WORLD);

    // set our fields
    m_training_dataset.num_samples_processed   = (long) header.train_proc;
    m_training_dataset.total_samples           = (long) header.train_total;
    m_testing_dataset.num_samples_processed    = (long) header.test_proc;
    m_testing_dataset.total_samples            = (long) header.test_total;
    m_validation_dataset.num_samples_processed = (long) header.validate_proc;
    m_validation_dataset.total_samples         = (long) header.validate_total;

    return true;
}
