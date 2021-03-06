#include "pooling_layer.h"
#include "conv_layer.h"

namespace sub_dl {

PoolingLayer::PoolingLayer(int input_dim, int output_dim, int pooling_x_dim, int pooling_y_dim,
    int feature_x_dim, int feature_y_dim) {

    _input_dim = input_dim;
    _output_dim = output_dim;
    _pooling_x_dim = pooling_x_dim;
    _pooling_y_dim = pooling_y_dim;
    _feature_x_dim = feature_x_dim;
    _feature_y_dim = feature_y_dim;
    _pooling_weights.resize(1, output_dim);
    _pooling_weights.assign_val();
    _pooling_bias.resize(1, output_dim);
    _pooling_bias.assign_val();
    _delta_pooling_weights.resize(1, output_dim);
    _delta_pooling_bias.resize(1, output_dim);
    _type = POOL;

}

void PoolingLayer::display() {
       std::cout << "-----------pooling layer-----------" << std::endl;
    _pooling_weights._display("_pooling_weights");
       std::cout << "-------------pooling bias -----------" << std::endl;
    _pooling_bias._display("_pooling_bias");
    std::cout << "-----------data-----------" << std::endl;
    for (int i = 0; i < _output_dim; i++) {
        _data[i]._display("data");
    }
       std::cout << "-----------pooling layer end-----------" << std::endl;
}

void PoolingLayer::_forward(Layer* pre_layer) {
    std::vector<matrix_double>().swap(_data);
    _pre_layer = pre_layer;
    for (int i = 0; i < _output_dim; i++) {
        matrix_double up_feature = (pre_layer->_data[i]
            .down_sample(_pooling_x_dim, _pooling_y_dim, AVG_POOLING)) 
            * _pooling_weights[0][i] 
            + _pooling_bias[0][i];
        // _data.push_back(sigmoid_m(up_feature));
        _data.push_back(up_feature);
    }
}

void PoolingLayer::_backward(Layer* nxt_layer) {
    if (nxt_layer->_type != ACT && nxt_layer->_type != CONV) {
        std::cerr << "Error layer type error before pooling!" << std::endl;
        exit(1);
    }

    std::vector<matrix_double>().swap(_errors);
    if (nxt_layer->_type == CONV) { 
        const ConvLayer* conv_layer = (ConvLayer*)(nxt_layer);
        for (int i = 0; i < _output_dim; i++) {
            matrix_double error(_feature_x_dim, _feature_y_dim);
            for (int j = 0; j < conv_layer->_output_dim; j++) {
                if (conv_layer->_conn_map[i][j]) {
                    matrix_double conv2d_vec = conv_layer->_errors[j]
                        .conv2d(conv_layer->_conv_kernels[i][j].rotate_180(), FULL);
                    error = error + conv2d_vec;
                }
            }
            error = error.dot_mul(sigmoid_m_diff(_data[i]));
            _errors.push_back(error);
        }
    } else {
        _errors = nxt_layer->_errors;
    }

    for (int i = 0; i < _output_dim; i++) {

        _delta_pooling_weights[0][i] = (_errors[i]
            .dot_mul(_pre_layer->_data[i].down_sample(_pooling_x_dim, _pooling_y_dim, AVG_POOLING)))
            .sum();
        _delta_pooling_bias[0][i] = _errors[i].sum();
    }
}

void PoolingLayer::_update_gradient(int opt_type, double learning_rate) {
    if (opt_type == SGD) {
        _pooling_weights.add(_delta_pooling_weights * learning_rate);
        _pooling_bias.add(_delta_pooling_bias * learning_rate);
    }
}

}
