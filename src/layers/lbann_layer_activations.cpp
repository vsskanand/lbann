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
// lbann_layer_activations .hpp .cpp - Basic activations: sigmoid, tanh, reLU
////////////////////////////////////////////////////////////////////////////////

#include "lbann/layers/lbann_layer_activations.hpp"
#include "lbann/utils/lbann_exception.hpp"

using namespace std;
using namespace El;

namespace lbann {

Activation* new_activation(activation_type act_fn, DataType param) {
  switch (act_fn) {
  case activation_type::SIGMOID:
    return new sigmoid_layer();
  case activation_type::TANH:
    return new tanh_layer();
  case activation_type::RELU:
    return new reLU_layer();
  case activation_type::ID:
    return new id_layer();
  case activation_type::LEAKY_RELU:
    return new leaky_reLU_layer(param);
#if 0
  case activation_type::SOFTPLUS:
    return new softplus_layer();
#else
  case activation_type::SMOOTH_RELU:
    return new smooth_reLU_layer();
#endif
  case activation_type::ELU:
    return new ELU_layer();
  default:
    throw lbann_exception("Unsupported activation type.");
  }
  return nullptr;  // Never reached.
}

// Activation class
DataType sigmoid_layer::sigmoid(const DataType& z)
{
    return (1.0 / (1.0 + exp(-z)));
}

DataType sigmoid_layer::sigmoidPrime(const DataType& z)
{
    DataType sigz = sigmoid(z);
    return sigz * (1 - sigz);
}

DataType tanh_layer::tanh(const DataType& z)
{
#ifdef __ICC
    // If using Intel compiler, use the MKL specific Tanh function
    return Tanh(z);
#else
    // Otherwise force the system to use the C++ version - glibc version is having problems with memory leaks
    return std::tanh(z);
#endif
}

DataType tanh_layer::tanhPrime(const DataType& z)
{
    float e = exp(2 * z);
    return ((e - 1) / (e + 1));
}

DataType reLU_layer::reLU(const DataType& z)
{
    return max((DataType) 0.0, z);
}

DataType reLU_layer::reLUPrime(const DataType& z)
{
    if (z > 0.0f) {
      return 1.0f;
    } else {
      return 0.0f;
    }
}

leaky_reLU_layer::leaky_reLU_layer(DataType leak) : leak(leak) {}

DataType leaky_reLU_layer::leaky_reLU(const DataType& z, DataType k)
{
    return max(k * z, z);
}

DataType leaky_reLU_layer::leaky_reLUPrime(const DataType& z, DataType k)
{
    if (z > 0.0f) {
      return 1.0f;
    } else {
      return k;
    }
}

#if 0
DataType softplus_layer::softplus(const DataType& z)
{
    return log(1.0 + exp(z)); // exp(z) can be very large and blow up
    // return log((exp(-z) + 1.0)/exp(-z)); // this can overflow or divided by zero
}

DataType softplus_layer::softplusPrime(const DataType& z)
{
    return (1.0 / (1.0 + exp(-z)));
}
#else
DataType smooth_reLU_layer::smooth_reLU(const DataType& z)
{
    return (z / (1.0 + exp(-z)));
}

DataType smooth_reLU_layer::smooth_reLUPrime(const DataType& z)
{
    DataType s = (1.0 / (1.0 + exp(-z)));
    return (s + z*s - z*s*s);
}
#endif

ELU_layer::ELU_layer(DataType alpha) : alpha(alpha) {}

DataType ELU_layer::elu(const DataType& z, DataType alpha) {
  if (z > 0) {
    return z;
  } else {
    return alpha * (std::exp(z) - 1);
  }
}

DataType ELU_layer::eluPrime(const DataType& z, DataType alpha) {
  if (z > 0) {
    return 1.0f;
  } else {
    // elu(z, alpha) + alpha
    return alpha * (std::exp(z) - 1) + alpha;
  }
}

////////////////////////////////////////////////////////////////////////////////
// There are three mechanisms for updating all fields of a distributed matrix
// 1) EntrywiseMap + independent reset of the bias row - Fastest (about ~30% faster)
// 2) IndexDependentMap (conditional application of function to mask bias row) - Slow
//        IndexDependentMap(m, 
//          (std::function<double(int,int,double)>)[m](int r, int c, double z)->
//          double{int bias_row = m.Height(); 
//                 if(r == bias_row - 1){return 1.0;}else{return sigmoid(z);}
//                 });
// 3) Dual nested loop over local matrix indices with global matrix lookups - Slow
////////////////////////////////////////////////////////////////////////////////
void sigmoid_layer::forwardProp(ElMat& m)
{
  EntrywiseMap(m, std::function<DataType(const DataType&)>(sigmoid));
}

void sigmoid_layer::backwardProp(ElMat& m)
{
  EntrywiseMap(m, std::function<DataType(const DataType&)>(sigmoidPrime));
}

void tanh_layer::forwardProp(ElMat& m)
{
  EntrywiseMap(m, std::function<DataType(const DataType&)>(tanh));
}

void tanh_layer::backwardProp(ElMat& m)
{
  EntrywiseMap(m, std::function<DataType(const DataType&)>(tanhPrime));
}

void reLU_layer::forwardProp(ElMat& m)
{
    EntrywiseMap(m, std::function<DataType(const DataType&)>(reLU));
}

void reLU_layer::backwardProp(ElMat& m)
{
  EntrywiseMap(m, std::function<DataType(const DataType&)>(reLUPrime));
}

void leaky_reLU_layer::forwardProp(ElMat& m)
{
  EntrywiseMap(m, std::function<DataType(const DataType&)>([this] (const DataType& x) -> DataType { return leaky_reLU(x, leak); }));
  //EntrywiseMap(m, std::function<DataType(const DataType&)>(leaky_reLU));
}

void leaky_reLU_layer::backwardProp(ElMat& m)
{
  EntrywiseMap(m, std::function<DataType(const DataType&)>([this] (const DataType& x) -> DataType { return leaky_reLUPrime(x, leak); }));
  //EntrywiseMap(m, std::function<DataType(const DataType&)>(leaky_reLUPrime));
}

#if 0
void softplus_layer::forwardProp(ElMat& m)
{
  EntrywiseMap(m, std::function<DataType(const DataType&)>(softplus));
}

void softplus_layer::backwardProp(ElMat& m)
{
  EntrywiseMap(m, std::function<DataType(const DataType&)>(softplusPrime));
}
#else
void smooth_reLU_layer::forwardProp(ElMat& m)
{
  EntrywiseMap(m, std::function<DataType(const DataType&)>(smooth_reLU));
}

void smooth_reLU_layer::backwardProp(ElMat& m)
{
  EntrywiseMap(m, std::function<DataType(const DataType&)>(smooth_reLUPrime));
}
#endif

void ELU_layer::forwardProp(ElMat& m) {
  EntrywiseMap(m, std::function<DataType(const DataType&)>(
                 [this] (const DataType& x) -> DataType { return elu(x, alpha); }));
}

void ELU_layer::backwardProp(ElMat& m) {
  EntrywiseMap(m, std::function<DataType(const DataType&)>(
                 [this] (const DataType& x) -> DataType { return eluPrime(x, alpha); }));
}

#include <string.h>
const string Activation::activation_name(activation_type id) {
  switch(id) {
    case activation_type::SIGMOID : return "sigmoid";
                   break;
    case activation_type::TANH : return "tanh";
                   break;
    case activation_type::RELU : return "relu";
                   break;
    case activation_type::ID : return "id";
                   break;
    case activation_type::LEAKY_RELU : return "leaky_relu";
                   break;
    case activation_type::SMOOTH_RELU : return "smooth_relu";
                   break;
    case activation_type::ELU : return "elu";
                   break;
    default : 
      stringstream err;
      //??? getting compile error, for reasons I don't understand
      //err << __FILE__ << " " << __LINE__ << " :: unknown activation_type: " << id;
      throw lbann_exception("unknown activation_type");
  }
}

}  // namespace lbann
