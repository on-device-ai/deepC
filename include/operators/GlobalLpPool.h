// Copyright 2018 The AITS DNNC Authors.All Rights Reserved.
//
// Licensed to the Apache Software Foundation(ASF) under one
// or more contributor license agreements.See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.See the License for the
// specific language governing permissionsand limitations
// under the License.
//
// This file is part of AITS DNN compiler maintained at
// https://github.com/ai-techsystems/dnnCompiler
//

#pragma once
#include "operators/baseOperator.h"
#include <math.h>
#include <string>
using namespace Eigen;

namespace dnnc {
/*! GlobalLpPool consumes an input tensor X and applies lp pool pooling across
 * the values in the same channel. This is equivalent to LpPool with kernel size
 * equal to the spatial dimension of input tensor.*/
template <typename T> class GlobalLpPool : public baseOperator<T, T, T> {
protected:
  int p = 2; /*!< p value of the Lp norm used to pool over the input data. */

public:
  GlobalLpPool(std::string name = "opGlobalLpPool", int p = 2)
      : baseOperator<T, T, T>(opGlobalLpPool, name) {
    this->p = p;
  }
  bool getAttribute(OPATTR attrName, int &obj) override {
    if (attrName == attr_p) {
      obj = p;
      return true;
    }
    return false;
  }
  bool setAttribute(OPATTR attrName, int obj) override {
    if (attrName == attr_p) {
      p = obj;
      return true;
    }
    return false;
  }
  tensor<T> compute(
      tensor<T> a /*!< [float,double]: ND tensor of shape ( NxCxD1xD2…Dk ).*/) {

    if (!(this->template type_check<float, double>(typeid(T))))
      throw std::invalid_argument(
          "Constrain input and output types to float tensors.");

    if ((a.rank() == 1) || (a.rank() == 2))
      return a;
    // Reshape ND tensor to 3D.
    size_t axis_left = 1;
    for (int i = 2; i < int(a.rank()); i++) {
      axis_left *= a.shape()[i];
    }
    std::vector<size_t> shape{a.shape()[0], a.shape()[1], axis_left};
    a.reshape(shape);
    // Make the axis other than N and C equal to 1.
    shape.pop_back();
    for (int i = 2; i < int(a.rank()); i++)
      shape.push_back(1);

    DNNC_EIGEN_TENSOR_MAP(eigenTensor, T, a);
    eigenTensor = eigenTensor.abs();
    eigenTensor = eigenTensor.pow(p);
    tensor<T> result(shape);
    Tensor<T, 2, RowMajor> eResult(a.shape()[0], a.shape()[1]);
    eResult = eigenTensor.sum(Eigen::array<int, 1>({2}));
    eResult = eResult.pow(pow(p, -1));
    result.load(eResult.data());
    return result;
  }
  /*!<
  \return Output data tensor from pooling across the input tensor. The output
  tensor has the same rank as the input. The first two dimensions of output
  shape are the same as the input (N x C), while the other dimensions are all 1.
  */
};
} // namespace dnnc
