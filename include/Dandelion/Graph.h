#ifndef DANDELION_GRAPH_H
#define DANDELION_GRAPH_H
#include <stdint.h>
#include <list>

#include "llvm/IR/Argument.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Value.h"

#include <fstream>
#include <ostream>

#include "json/json.h"

#include "Dandelion/Edge.h"
#include "Dandelion/Node.h"
#include "iterator_range.h"

namespace dandelion {

using InstructionList = std::list<std::unique_ptr<InstructionNode>>;
using ArgumentList = std::list<std::unique_ptr<ArgumentNode>>;
using BasicBlockList = std::list<std::unique_ptr<SuperNode>>;
using GlobalValueList = std::list<GlobalValueNode>;
using ConstIntList = std::list<ConstIntNode>;
using LoopNodeList = std::list<std::unique_ptr<LoopNode>>;
using EdgeList = std::list<std::unique_ptr<Edge>>;
using Port = std::pair<Node *, PortID>;

class Graph {
   public:

   private:
    // Node information
    NodeInfo graph_info;

    // List of elements inside each graph
    InstructionList inst_list;
    ArgumentList arg_list;
    BasicBlockList super_node_list;
    GlobalValueList glob_list;
    ConstIntList const_list;

    // Splitcall for the function
    std::unique_ptr<SplitCallNode> split_call;

    // List of the edges between nodes inside the graph
    EdgeList edge_list;

    bool graph_empty;
    llvm::Function *function_ptr;
    llvm::raw_ostream &outCode;

    // Memory units inside each graph
    std::unique_ptr<MemoryNode> memory_unit;

    // Loop nodes
    LoopNodeList loop_nodes;

    // Out interface
    Node *out_node;

    // Keep track of nodes and values
    std::map<llvm::Value *, Node *> map_value_node;

   public:
    explicit Graph(NodeInfo _n_info)
        : graph_info(_n_info),
          split_call(
              std::make_unique<SplitCallNode>(NodeInfo(0, "InputSplitter"))),
          memory_unit(std::make_unique<MemoryNode>(NodeInfo(0, "MemCtrl"))),
          graph_empty(false),
          outCode(llvm::outs()),
          function_ptr(nullptr),
          out_node(nullptr) {}
    explicit Graph(NodeInfo _n_info, llvm::raw_ostream &_output)
        : graph_info(_n_info),
          split_call(
              std::make_unique<SplitCallNode>(NodeInfo(0, "InputSplitter"))),
          memory_unit(std::make_unique<MemoryNode>(NodeInfo(0, "MemCtrl"))),
          graph_empty(false),
          outCode(_output),
          function_ptr(nullptr),
          out_node(nullptr) {}
    explicit Graph(NodeInfo _n_info, llvm::raw_ostream &_output,
                   llvm::Function *_fn)
        : graph_info(_n_info),
          split_call(
              std::make_unique<SplitCallNode>(NodeInfo(0, "InputSplitter"))),
          memory_unit(std::make_unique<MemoryNode>(NodeInfo(0, "MemCtrl"))),
          graph_empty(false),
          outCode(_output),
          function_ptr(_fn),
          out_node(nullptr) {}

    void init(BasicBlockList &, InstructionList &, ArgumentList &,
              GlobalValueList &, ConstIntList &, EdgeList &);

    void doInitialization();
    void doFinalization();

    void printGraph(PrintType);

    bool isEmpty() { return graph_empty; }
    MemoryNode *getMemoryUnit() const { return memory_unit.get(); }

    // InstructionList *getInstructionList();
    auto instList_begin() { return this->inst_list.cbegin(); }
    auto instList_end() { return this->inst_list.cend(); }
    auto instructions() {
        return helpers::make_range(instList_begin(), instList_end());
    }

    auto funarg_begin() { return this->arg_list.cbegin(); }
    auto funarg_end() { return this->arg_list.cend(); }
    auto args() { return helpers::make_range(funarg_begin(), funarg_end()); }

    auto edge_begin() { return this->edge_list.cbegin(); }
    auto edge_end() { return this->edge_list.cend(); }
    auto edges() { return helpers::make_range(edge_begin(), edge_end()); }

    void insertInstruction(llvm::Instruction &);
    void setFunction(llvm::Function *);
    SuperNode *insertSuperNode(llvm::BasicBlock &);
    InstructionNode *insertBinaryOperatorNode(llvm::BinaryOperator &);
    InstructionNode *insertIcmpOperatorNode(llvm::ICmpInst &);
    InstructionNode *insertBranchNode(llvm::BranchInst &);
    InstructionNode *insertPhiNode(llvm::PHINode &);
    InstructionNode *insertAllocaNode(llvm::AllocaInst &);
    InstructionNode *insertGepNode(llvm::GetElementPtrInst &);
    InstructionNode *insertLoadNode(llvm::LoadInst &);
    InstructionNode *insertStoreNode(llvm::StoreInst &);
    InstructionNode *insertReturnNode(llvm::ReturnInst &);
    InstructionNode *insertCallNode(llvm::CallInst &);
    ArgumentNode *insertFunctionArgument(llvm::Argument &);
    GlobalValueNode *insertFunctionGlobalValue(llvm::GlobalValue &);
    ConstIntNode *insertConstIntNode(llvm::ConstantInt &);

    LoopNode *insertLoopNode(std::unique_ptr<LoopNode>);

    void breakEdge(Node *, Node *, Node *);

    Edge *insertEdge(Edge::EdgeType, Port src, Port dst);
    bool edgeExist(Port _node_src, Port _node_dst);
    void removeEdge(Node *, Node *);
    Edge *findEdge(const Port _src, const Port _dst) const;
    Edge *findEdge(const Node *_src, const Node * _dst) const;

    Edge *insertMemoryEdge(Edge::EdgeType, Port src, Port dst);

    SplitCallNode *getSplitCall() const { return split_call.get(); }

    void setOutputNode(Node *_n) { out_node = _n; }

   protected:
    // General print functions with accepting print type
    void printFunctionArgument(PrintType);
    void printBasicBlocks(PrintType);
    void printInstructions(PrintType);
    void printMemoryModules(PrintType);
    void printBasickBlockPredicateEdges(PrintType);
    void printBasickBLockInstructionEdges(PrintType);
    void printPhiNodesConnections(PrintType);
    void printMemInsConnections(PrintType _pt);
    void printDatadependencies(PrintType);
    void printClosingclass(PrintType);
    void printLoopEndingDependencies(PrintType _pt);
    void printLoopBranchEdges(PrintType);
    void printLoopHeader(PrintType);
    void printOutPort(PrintType);

    // Scala specific functions
    void printScalaHeader(std::string, std::string);
    void printScalaFunctionHeader();
    void printScalaInputSpliter();
    void printScalaMainClass();
};
}

#endif  // end of DANDDELION_GRAPH_H
