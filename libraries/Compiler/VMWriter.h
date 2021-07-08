#pragma once
#include <ostream>
#include <vector>
#include <map>

namespace Compiler
{
    enum class Segment { CONSTANT, ARG, LOCAL, STATIC, THIS, THAT, POINTER, TEMP };

    enum class Command { ADD, SUB, NEG, GT, LT, AND, OR, NOT, EQ };
    class VMWriter
    {
    public:
        VMWriter() : m_stream{ nullptr } {}
        VMWriter(std::ostream* stream) : m_stream{ stream } {}
        void writePush(Segment segment, int index)
        {
            write("push " + segmentToString(segment) + " " + std::to_string(index));
        }
        void writePop(Segment segment, int index)
        {
            write("pop " + segmentToString(segment) + " " + std::to_string(index));
        }
        void writeArithmetic(Command command)
        {
            write(commandToString(command));
        }
        void writeLabel(const std::string& label)
        {
            write("label " + label);
        }
        void writeGoto(const std::string& label)
        {
            write("goto " + label);
        }
        void writeIf(const std::string& label)
        {
            write("if-goto " + label);
        }
        void writeCall(const std::string& name, int nArgs)
        {
            write("call " + name + " " + std::to_string(nArgs));
        }
        void writeFunction(const std::string& name, int nLocals)
        {
            write("function " + name + " " + std::to_string(nLocals));
        }
        void writeReturn() { write("return"); }

        static std::string segmentToString(Segment segment)
        {
            if (segment == Segment::CONSTANT) return "constant";
            else if (segment == Segment::ARG) return "argument";
            else if (segment == Segment::LOCAL) return "local";
            else if (segment == Segment::STATIC) return "static";
            else if (segment == Segment::THIS) return "this";
            else if (segment == Segment::THAT) return "that";
            else if (segment == Segment::POINTER) return "pointer";
            else return "temp";
        };
        static std::string commandToString(Command command)
        {
            if (command == Command::ADD) return "add";
            else if (command == Command::SUB) return "sub";
            else if (command == Command::NEG) return "neg";
            else if (command == Command::GT) return "gt";
            else if (command == Command::LT) return "lt";
            else if (command == Command::AND) return "and";
            else if (command == Command::OR) return "or";
            else if (command == Command::NOT) return "not";
            else if (command == Command::EQ) return "eq";
            else return "not";
        };

        std::string newLabelId() { return std::to_string(m_labelIndex++); }
        void resetLabelIndex() { m_labelIndex = 0; }
        virtual void clear()
        {
            m_stream->clear();
            m_labelIndex = 0;
        }
    private:
        virtual void write(const std::string line) { if (m_stream) *m_stream << line << '\n'; }
        std::ostream* m_stream{};
        int m_labelIndex{};
    };
}