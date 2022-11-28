#include "model.h"
#include "logging.h"
#include "error.h"

#include <queue>

// Construct Tensor from protobuf
Tensor::Tensor(const ::onnx::TensorProto& tensor) {
    // Name
    m_name = tensor.name();

    // Dimension
    m_size = 1;
    for (auto dim : tensor.dims()) {
        m_dims.push_back(dim);
        m_size *= dim;
    }
    m_value = (float*)calloc(m_size, sizeof(float));
    if (m_value == NULL) {
        FatalError("Insufficient memory");
    }

    // Value
    if (tensor.data_type() == ::onnx::TensorProto::FLOAT) {
        if (tensor.float_data_size() != 0) {
            // Float data
            if ((size_t)tensor.float_data_size() > m_size) {
                FatalError("Initializer " + m_name + " larger than dimension " + std::to_string(m_size));
            }
            for (size_t i = 0;i < (size_t)tensor.float_data_size();i++) {
                m_value[i] = tensor.float_data(i);
            }
        } else {
            // Raw data
            std::string bytes = tensor.raw_data();
            if (bytes.size() / sizeof(float) > m_size) {
                FatalError("Initializer " + m_name + " larger than dimension " + std::to_string(m_size));
            }
            memcpy(m_value, bytes.c_str(), bytes.size());
        }
    } else {
        LogWarning("Initializer tensor " + m_name + " has unsupported type " + std::to_string(tensor.data_type()) + " and set to 0");
    }
}

// Cleanup tensor
Tensor::~Tensor() {
    free(m_value);
}

// Constructor for ONNXModel
ONNXModel::ONNXModel(const std::string& filename) {
    LogInfo("Loading model " + filename);

    // Find file size
    FILE* file = fopen(filename.c_str(), "rb");
    if (file == NULL) {
        FatalError(strerror(errno));
    }
    fseek(file, 0, SEEK_END);
    size_t filesize = ftell(file);
    rewind(file);

    // Read into buffer
    void* buffer = malloc(filesize);
    fread(buffer, filesize, 1, file);
    fclose(file);

    // Construct model object
    bool ret = m_model.ParseFromArray(buffer, filesize);
    if (!ret) {
        FatalError("Cannot load model " + filename);
    }

    // Free buffer
    free(buffer);

    // Other init operation
    loadModel();
}

// Loading data from model
void ONNXModel::loadModel() {
    m_graph = m_model.graph();
    m_nodeCount = m_graph.node_size();
    LogInfo("IR version " + std::to_string(m_model.ir_version()));
    LogInfo("Graph name " + m_graph.name());

    // Initializer
    for (const auto& init : m_graph.initializer()) {
        m_initializers.emplace(init.name(), init);
    }

    // Load nodes
    genDepGraph();
}

// Generate dependency graph
void ONNXModel::genDepGraph() {
    // Map from output name to node
    std::map<std::string, size_t> rev_output;
    for (size_t i = 0;i < m_nodeCount;i++) {
        for (const auto& out : m_graph.node(i).output()) {
            if (rev_output.count(out)) {
                FatalError("Dual output into " + out);
            }
            rev_output[out] = i;
        }
    }

    // Test input
    m_nodeDep.assign(m_nodeCount, std::vector<size_t>{});
    std::vector<size_t> incoming_edges(m_nodeCount, 0);
    for (size_t i = 0;i < m_nodeCount;i++) {
        for (const auto& in : m_graph.node(i).input()) {
            if (rev_output.count(in)) {
                size_t prev = rev_output[in];
                m_nodeDep[prev].push_back(i);
                incoming_edges[i]++;
            }
        }
    }

    // Sort
    std::queue<size_t> q;
    for (size_t i = 0;i < m_nodeCount;i++) {
        if (incoming_edges[i] == 0) {
            q.push(i);
        }
    }
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        m_nodePerm.push_back(u);

        for (const auto& v : m_nodeDep[u]) {
            incoming_edges[v]--;
            if (incoming_edges[v] == 0) {
                q.push(v);
            }
        }
    }

    // Print resulting nodes
    for (size_t i = 0;i < m_nodeCount;i++) {
        const auto& node = m_graph.node(m_nodePerm[i]);
        LogInfo(node.name() + " (" + node.op_type() + ")");
    }
}
