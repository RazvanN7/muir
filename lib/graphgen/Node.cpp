#define DEBUG_TYPE "graphgen"

#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "Common.h"
#include "Dandelion/Node.h"

#include <algorithm>
#include <iostream>
#include <sstream>

using namespace std;
using namespace llvm;
using namespace dandelion;
using namespace helpers;

//===----------------------------------------------------------------------===//
//                            Node Class
//===----------------------------------------------------------------------===//

void Node::addDataInputPort(Node *n) {
    port_data.data_input_port.emplace_back(n);
}
void Node::addDataOutputPort(Node *n) {
    port_data.data_output_port.emplace_back(n);
}

void Node::addControlInputPort(Node *n) {
    port_control.control_input_port.emplace_back(n);
}
void Node::addControlOutputPort(Node *n) {
    port_control.control_output_port.emplace_back(n);
}

uint32_t Node::returnDataOutputPortIndex(Node &_node) {
    return std::distance(this->port_data.data_output_port.begin(),
                         find(this->port_data.data_output_port.begin(),
                              this->port_data.data_output_port.end(), &_node));
}

uint32_t Node::returnDataInputPortIndex(Node &_node) {
    return std::distance(this->port_data.data_input_port.begin(),
                         find(this->port_data.data_input_port.begin(),
                              this->port_data.data_input_port.end(), &_node));
}

//===----------------------------------------------------------------------===//
//                            SuperNode Class
//===----------------------------------------------------------------------===//

void SuperNode::addInstruction(InstructionNode *node) {
    this->instruction_list.push_back(node);
}

void SuperNode::addPhiInstruction(PhiSelectNode *node) {
    this->phi_list.push_back(node);
}

std::string SuperNode::printDefinition(PrintType pt) {
    string _text;
    string _name(this->getName());
    switch (pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text =
                "  val $name = Module(new $type(NumInputs = $num_in, NumOuts = "
                "$num_out, BID = $bid))\n\n";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$num_in", this->numControlInputPort());
            helperReplace(_text, "$num_out",
                          std::to_string(this->numControlOutputPort()));
            helperReplace(_text, "$bid", this->getID());
            if (this->getNumPhi())
                helperReplace(_text, "$type", "BasicBlockNoMaskNode");
            else
                helperReplace(_text, "$type", "BasicBlockNode");

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string SuperNode::printInputEnable(PrintType pt, uint32_t _id) {
    string _text;
    string _name(this->getName());
    switch (pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.predicateIn($id)";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _id);

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string SuperNode::printOutputEnable(PrintType pt, uint32_t _id) {
    string _text;
    string _name(this->getName());
    switch (pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.Out($id)";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _id);

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string SuperNode::printMaskOutput(PrintType pt, uint32_t _id) {
    string _text;
    string _name(this->getName());
    switch (pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.MaskBB($id)";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _id);

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

//===----------------------------------------------------------------------===//
//                            MemoryUnitNode Class
//===----------------------------------------------------------------------===//

void MemoryUnitNode::addReadMemoryReqPort(Node *const n) {
    read_port_data.memory_req_port.emplace_back(n);
}
void MemoryUnitNode::addReadMemoryRespPort(Node *const n) {
    read_port_data.memory_resp_port.emplace_back(n);
}

void MemoryUnitNode::addWriteMemoryReqPort(Node *const n) {
    write_port_data.memory_req_port.emplace_back(n);
}
void MemoryUnitNode::addWriteMemoryRespPort(Node *const n) {
    write_port_data.memory_resp_port.emplace_back(n);
}

std::string MemoryUnitNode::printDefinition(PrintType pt) {
    string _text;
    string _name(this->getName());
    switch (pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text =
                "  val $name = Module(new $reg_type(ID=$id, Size=$size, "
                "NReads=$num_read, NWrites=$num_write))\n"
                "\t\t (WControl=new WriteMemoryController(NumOps=$read_num_op, "
                "BaseSize=$read_base_size, NumEntries=$read_num_entries))\n"
                "\t\t (RControl=new ReadMemoryController(NumOps=$write_num_op, "
                "BaseSize=$write_base_size, "
                "NumEntries=$write_num_entries)))\n\n"
                "  io.MemReq <> $name.MemReq\n"
                "  $name.io.MemResp <> io.MemResp\n\n";
            ;
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$reg_type", "UnifiedController");
            helperReplace(_text, "$id", std::to_string(this->getID()));

            // TODO this part can be parametrize using config file
            helperReplace(_text, "$size", std::to_string(32));
            helperReplace(_text, "$num_read",
                          std::to_string(this->numReadDataInputPort()));
            helperReplace(_text, "$num_write",
                          std::to_string(this->numWriteDataOutputPort()));
            helperReplace(_text, "$read_num_op",
                          std::to_string(this->numReadDataInputPort()));
            helperReplace(_text, "$read_base_size", std::to_string(2));
            helperReplace(_text, "$read_num_entries",
                          std::to_string(this->numReadDataInputPort()));
            helperReplace(_text, "$write_num_op",
                          std::to_string(this->numWriteDataOutputPort()));
            helperReplace(_text, "$write_base_size", std::to_string(2));
            helperReplace(_text, "$write_num_entries",
                          std::to_string(this->numReadDataInputPort()));

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

//===----------------------------------------------------------------------===//
//                            CallSpliter Class
//===----------------------------------------------------------------------===//

std::string SplitCallNode::printDefinition(PrintType _pt) {
    string _text("");
    string _name(this->getName());

    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text =
                "  val $name = Module(new $type(List($<input_vector>)))\n"
                "  $name.io.In <> io.in\n\n";

            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$type", "SplitCall");
            helperReplace(_text, "$id", std::to_string(this->getID()));
            helperReplace(
                _text, "$<input_vector>",
                std::vector<uint32_t>(this->numDataOutputPort(), XLEN), ",");
            // TODO: uncomment if you update the list shape.
            // this->num_ports, ",");

            break;
        default:
            assert(!"Don't support!");
    }
    return _text;
}

std::string SplitCallNode::printOutputEnable(PrintType _pt, uint32_t _id) {
    string _name(this->getName());
    std::replace(_name.begin(), _name.end(), '.', '_');
    string _text;
    switch (_pt) {
        case PrintType::Scala:
            _text = "$name.io.Out.enable";
            helperReplace(_text, "$name", _name.c_str());
            break;
        default:
            break;
    }

    return _text;
}


std::string SplitCallNode::printOutputData(PrintType _pt, uint32_t _idx) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.Out.data(\"field$id\")";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _idx);

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}


//===----------------------------------------------------------------------===//
//                            BranchNode Class
//===----------------------------------------------------------------------===//

std::string BranchNode::printOutputEnable(PrintType _pt, uint32_t _id) {
    string _name(this->getName());
    std::replace(_name.begin(), _name.end(), '.', '_');
    string _text;
    switch (_pt) {
        case PrintType::Scala:
            _text = "$name.io.Out($id)";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _id);
            break;
        default:
            break;
    }

    return _text;
}


std::string BranchNode::printInputData(PrintType _pt, uint32_t _id) {
    string _name(this->getName());
    std::replace(_name.begin(), _name.end(), '.', '_');
    string _text;
    switch (_pt) {
        case PrintType::Scala:
            _text = "$name.io.CmpIO($id)";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _id);
            break;
        default:
            break;
    }

    return _text;
}

//===----------------------------------------------------------------------===//
//                            Instruction Class
//===----------------------------------------------------------------------===//
//
/**
 * Returning address of the parent instruction
 */
Instruction *InstructionNode::getInstruction() {
    return this->parent_instruction;
}

ConstantInt *ConstIntNode::getConstantParent() {
    return this->parent_const_int;
}

//===----------------------------------------------------------------------===//
//                            ArgumentNode Class
//===----------------------------------------------------------------------===//
Argument *ArgumentNode::getArgumentValue() { return this->parent_argument; }

std::string ArgumentNode::printDefinition(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text =
                "  val $name = Module(new $type(NumOuts = "
                "$num_out, ID = $id))\n\n";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$num_out",
                          std::to_string(this->numDataOutputPort()));
            helperReplace(_text, "$id", this->getID());
            helperReplace(_text, "$type", "ArgumentNode");

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string ArgumentNode::printInputData(PrintType _pt, uint32_t _idx) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.InData($id)";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _idx);

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}


std::string ArgumentNode::printOutputData(PrintType _pt, uint32_t _idx) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.Out($id)";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _idx);

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

//===----------------------------------------------------------------------===//
//                            GlobalNode Class
//===----------------------------------------------------------------------===//
GlobalValue *GlobalValueNode::getGlobalValue() { return this->parent_glob; }

BasicBlock *SuperNode::getBasicBlock() { return this->basic_block; }

//===----------------------------------------------------------------------===//
//                            BinaryOperatorNode Class
//===----------------------------------------------------------------------===//

std::string BinaryOperatorNode::printDefinition(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text =
                "  val $name = Module(new $type(NumOuts = "
                "$num_out, ID = $id, opCode = \"$opcode\")(sign=false))\n\n";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$num_out",
                          std::to_string(this->numDataOutputPort()));
            helperReplace(_text, "$id", this->getID());
            helperReplace(_text, "$type", "ComputeNode");

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string BinaryOperatorNode::printInputEnable(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.enable";
            helperReplace(_text, "$name", _name.c_str());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string BinaryOperatorNode::printOutputData(PrintType _pt, uint32_t _id) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.Out($id)";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _id);

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string BinaryOperatorNode::printInputData(PrintType _pt, uint32_t _idx) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            if (_idx == 0)
                _text = "$name.io.Left";
            else
                _text = "$name.io.Right";
            helperReplace(_text, "$name", _name.c_str());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

//===----------------------------------------------------------------------===//
//                            ICMPNode Class
//===----------------------------------------------------------------------===//

std::string IcmpNode::printDefinition(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text =
                "  val $name = Module(new $type(NumOuts = "
                "$num_out, ID = $id, opCode = \"$opcode\")(sign=false))\n\n";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$num_out",
                          std::to_string(this->numDataOutputPort()));
            helperReplace(_text, "$id", this->getID());
            helperReplace(_text, "$type", "ComputeNode");

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string IcmpNode::printInputEnable(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.enable";
            helperReplace(_text, "$name", _name.c_str());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string IcmpNode::printInputData(PrintType _pt, uint32_t _idx) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            if (_idx == 0)
                _text = "$name.io.Left";
            else
                _text = "$name.io.Right";
            helperReplace(_text, "$name", _name.c_str());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string IcmpNode::printOutputData(PrintType _pt, uint32_t _id) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.Out($id)";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _id);

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

//===----------------------------------------------------------------------===//
//                            BranchNode Class
//===----------------------------------------------------------------------===//

std::string BranchNode::printDefinition(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            if (this->numDataInputPort() > 0) {
                _text = "  val $name = Module(new $type(ID = $id))\n\n";
                helperReplace(_text, "$type", "CBranchNode");
                helperReplace(_text, "$num_out",
                              std::to_string(this->numControlOutputPort()));
            } else
                helperReplace(_text, "$type", "UBranchNode");

            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", this->getID());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string BranchNode::printInputEnable(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.enable";
            helperReplace(_text, "$name", _name.c_str());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

//===----------------------------------------------------------------------===//
//                            PhiSelectNode Class
//===----------------------------------------------------------------------===//

std::string PhiSelectNode::printDefinition(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text =
                "  val $name = Module(new $type(NumInputs = $num_in, "
                "NumOutputs = $num_out, ID = $id))\n\n";
            helperReplace(_text, "$type", "PhiNode");
            helperReplace(_text, "$num_in",
                          std::to_string(this->numDataInputPort()));
            helperReplace(_text, "$num_out",
                          std::to_string(this->numDataOutputPort()));

            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", this->getID());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string PhiSelectNode::printInputEnable(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.enable";
            helperReplace(_text, "$name", _name.c_str());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string PhiSelectNode::printInputData(PrintType _pt, uint32_t _id) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.InData($id)";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _id);

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}


std::string PhiSelectNode::printOutputData(PrintType _pt, uint32_t _id) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.Out($id)";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _id);

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string PhiSelectNode::printMaskInput(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.Mask";
            helperReplace(_text, "$name", _name.c_str());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

//===----------------------------------------------------------------------===//
//                            ReturnNode Class
//===----------------------------------------------------------------------===//
//
std::string ReturnNode::printDefinition(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text =
                "  val $name = Module(new $type(retTypes=List($<input_list>), "
                "ID = $id))\n\n";
            helperReplace(_text, "$type", "RetNode");
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", this->getID());
            helperReplace(_text, "$<input_list>",
                          std::vector<uint32_t>(this->numDataInputPort(), 32),
                          ",");

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string ReturnNode::printInputEnable(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.enable";
            helperReplace(_text, "$name", _name.c_str());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string ReturnNode::printInputData(PrintType _pt, uint32_t _id) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.in.data(\"field$id\")";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _id);

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string ReturnNode::printOutputData(PrintType _pt, uint32_t _id) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.Out";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _id);

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

//===----------------------------------------------------------------------===//
//                            LoadNode Class
//===----------------------------------------------------------------------===//

void LoadNode::addReadMemoryReqPort(Node *const _nd) {
    this->read_port_data.memory_req_port.push_back(_nd);
}
void LoadNode::addReadMemoryRespPort(Node *const _nd) {
    this->read_port_data.memory_resp_port.push_back(_nd);
}

//===----------------------------------------------------------------------===//
//                            ConstantNode Class
//===----------------------------------------------------------------------===//

std::string ConstIntNode::printOutputData(PrintType _pt, uint32_t _id) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.Out($id)";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _id);

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}
