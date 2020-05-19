#pragma once
#include <duktape.h>
#include <map>
#include <string>
#include <iostream>

class DebugStack
{
    public:
        DebugStack(const std::string& label, duk_context * c, int expectedChange = 0)
                : _ctx(c)
                , _label(label)
        {
            //std::cout << "<" << label << ">" << std::endl;
            if (_pile.find(label) == _pile.end())
            {
                _pile[label] = 0;
            }
            _pile[label]++;
            _start = duk_get_top(_ctx);
            _start += expectedChange;
        }
        void addExpected(int amount)
        {
            _start += amount;
        }
        ~DebugStack()
        {
            //std::cout << "</" << _label << ">" << std::endl;
            int end = duk_get_top(_ctx);
            if (_start != end)
            {
                std::cerr << "Stack corruption detected!! " << _label << " " << _pile[_label] << ": Start " << _start <<", End: " << end << std::endl;
            }
            _pile[_label]--;
        }
    private:
        int _start = 0;
        std::string _label;
        duk_context * _ctx;
        std::map<std::string, int> _pile;
};