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
// lbann_callback_save_images .hpp .cpp - Callbacks to save images, currently used in autoencoder
////////////////////////////////////////////////////////////////////////////////

#ifndef LBANN_CALLBACKS_CALLBACK_SAVE_IMAGES_HPP_INCLUDED
#define LBANN_CALLBACKS_CALLBACK_SAVE_IMAGES_HPP_INCLUDED

#include "lbann/callbacks/lbann_callback.hpp"
#include "lbann/data_readers/lbann_data_reader.hpp"

namespace lbann {

/**
 * Save images to file
 */
class lbann_callback_save_images : public lbann_callback {
public:
  /**
   * @note  this will be deprecated 
   * @param image_dir directory to save images
   * @param num_images number of images to save
   */
  lbann_callback_save_images(std::string image_dir, size_t num_images=10) :
    lbann_callback(), m_image_dir(image_dir), m_num_images(num_images) {
      set_name("save_images");
    }
  /**
   * @note This uses data reader, previous (above) will be deprecated
   * @param data reader type e.g., imagenet, mnist, cifar10....
   * @param image_dir directory to save image
   * @param image extension e.g., jpg, png, pgm......
   */
  lbann_callback_save_images(DataReader* reader,std::string image_dir,std::string extension="jpg") :
    lbann_callback(), m_reader(reader),m_image_dir(image_dir),m_extension(extension) {
      set_name("save_images");
    }
  void on_phase_end(model* m);

private:
  std::string m_image_dir; //directory to save image
  std::string m_extension; //image extension; pgm, jpg, png etc
  Int m_num_images; // num of images to save
  DataReader* m_reader;
  void save_images(ElMat* input, ElMat* output,int phase);
  void save_image(ElMat* input, ElMat* output); //this version uses data reader
};

}  // namespace lbann

#endif  // LBANN_CALLBACKS_CALLBACK_SAVE_IMAGES_HPP_INCLUDED
