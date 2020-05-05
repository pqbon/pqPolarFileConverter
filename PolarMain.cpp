#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdint>
#include <cassert>
#include <string>
#include <string_view>
#include <vector>
#include <deque>
#include <map>
#include <utility>
#include <optional>

#include "PolarFile.h"

using namespace std::literals::string_literals;
using namespace std::literals::string_view_literals;

void display_help(std::string const& name_);

using ArgStrings = std::vector<std::string>;
ArgStrings process_arguments(int argc_, char* const argv_[]);

int main(int argc, char* const argv[]) {

    if (argc < 3) {
        display_help(argv[0]);
        std::exit(666);
    }

    ArgStrings const args { process_arguments(argc, argv) };

    std::string const input_filename { args.at(1) };
    std::string const output_filename { args.at(2) };

    std::cout << "Inputfile => " << input_filename << " Outpuffile => " << output_filename << std::endl;

    Sailing::PolarFiles::PolarFile new_polar;
    new_polar.import_polar(input_filename);
    new_polar.export_polar(output_filename);

    //process_input_polar(input_filename);
}

void display_help(std::string const& name_) {
    std::cout << name_ << " => Polar File Conversion for Expedition"s << std::endl << std::endl;
    std::cout << "Usage: "s << name_ << " <input file> <outputfile>"s << std::endl;
    std::cout << std::endl;
}

ArgStrings process_arguments(int argc_, char* const argv_[]) {
    ArgStrings new_args {};

    new_args.resize(argc_);

    for (int idx { 0 }; idx < argc_; ++idx) {
        new_args.at(idx) = argv_[idx];
    }

    return new_args;
}

bool Sailing::PolarFiles::PolarFile::my_begins_with(std::string_view const str, std::string_view const prefix) {
    bool match { true };

    size_t const str_sz { str.size() };
    size_t const prefix_sz { prefix.size() };

    if (str_sz < prefix_sz) {
        match = false;
    }

    int idx {};
    for (idx = 0; idx < prefix_sz; ++idx) {
        if (std::tolower(str[idx]) != std::tolower(prefix[idx])) {
            match = false;
            break;
        }
    }

    return match;
}

std::vector<Sailing::PolarFiles::str_ptr> Sailing::PolarFiles::PolarFile::break_strings(Sailing::PolarFiles::str_ptr input, char const cut_char) {
    std::vector<std::shared_ptr<std::string>> results {};
    size_t str_start { 0u };
    size_t const input_sz { input->size() };
    while (str_start < input_sz) {
        size_t str_end { input->find(cut_char, str_start) };
        if (str_end == std::string::npos) {
            str_end = input_sz;
        }
        std::shared_ptr<std::string> piece { std::make_shared<std::string>(input->substr(str_start, str_end - str_start)) };
        results.push_back(piece);
        str_start = str_end + 1;
    }

    return results;
}

std::vector<float> Sailing::PolarFiles::PolarFile::break_strings_float(Sailing::PolarFiles::str_ptr input, char const cut_char) {
    std::vector<float> results {};
    size_t str_start { 0u };
    size_t const input_sz { input->size() };
    while (str_start < input_sz) {
        size_t str_end { input->find(cut_char, str_start) };
        if (str_end == std::string::npos) {
            str_end = input_sz;
        }
        std::string tstr { input->substr(str_start, str_end - str_start) };
        if (std::isdigit(tstr[0]) != 0) {
            float val { static_cast<float>(atof(tstr.c_str())) };
            results.push_back(val);
        }
        str_start = str_end + 1;
    }

    return results;
}

Sailing::PolarFiles::TwsCurve_ptr Sailing::PolarFiles::PolarFile::build_pairs(std::vector<float> const& tws, std::vector<float> const& bsp) {
    TwsCurve_ptr result { std::make_shared<TwsCurve>() };

    size_t const twa_sz { tws.size() };
    size_t const bsp_sz { bsp.size() };
    assert(twa_sz == bsp_sz);

    for (size_t idx { 0u }; idx < twa_sz; ++idx) {
        float twsT { tws.at(idx) };
        float bspT { bsp.at(idx) };

        FloatPair_ptr tws_bsp_pair { std::make_shared<FloatPair>(std::make_pair(twsT, bspT)) };
        result->push_back(tws_bsp_pair);
    }

    return result;
}

void Sailing::PolarFiles::PolarFile::import_polar(std::string const& input_polar_fn) {
    std::shared_ptr<std::ifstream> polar_file { std::make_shared<std::ifstream>(input_polar_fn) };

    if (polar_file->is_open() == false) {
        std::cerr << "ERROR: Unable to open file  " << input_polar_fn << std::endl;
        return;
    }

    std::shared_ptr<std::string> tline { std::make_shared<std::string>() };
    std::getline(*polar_file, *tline);
    //1st line must start with table_deco "TWA/TWS
    bool const match0 { my_begins_with(*tline, table_deco0) };
    bool const match1 { my_begins_with(*tline, table_deco1) };

    if ((match0 == false) && (match1 == false)) {
        return;
    }

    size_t cut_pos { (match0 == true) ? table_deco0.size() : table_deco1.size() };

    char const cut_char { tline->at(cut_pos) };
    std::cout << "Cut Char: '" << cut_char << "' 0x" << std::hex << static_cast<int>(cut_char) << std::endl;

    //put int object
    tws_list = std::make_shared<std::vector<float>>(break_strings_float(tline, cut_char));


    for (std::shared_ptr<std::string> line { std::make_shared<std::string>() }; std::getline(*polar_file, *line);) {
        std::cout << *line << std::endl;

        std::shared_ptr<std::vector<float>> row { std::make_shared<std::vector<float>>(break_strings_float(line, cut_char)) };

        float const twa { row->at(0) };
        twa_list.push_back(twa);

        old_polar_map[twa] = row;
    }

    //Put in onject...
    polar = std::make_shared<std::vector<TwsCurve_ptr>>();
    size_t const tws_list_sz { tws_list->size() };
    size_t const twa_list_sz { twa_list.size() };
    for (size_t idx { 0u }; idx < tws_list_sz; ++idx) {
        float tws { tws_list->at(idx) };
        std::vector<float> new_row {};
        for (size_t jdx { 0u }; jdx < twa_list_sz; ++jdx) {
            float key { twa_list.at(jdx) };
            float val { old_polar_map[key]->at(idx + 1) };
            new_row.push_back(val);
        }
        TwsCurve_ptr polar_curves { build_pairs(twa_list, new_row) };

        polar->push_back(polar_curves);
    }

    return;
}

void Sailing::PolarFiles::PolarFile::export_polar(std::string const& output_polar_fn) {
    std::ofstream output_polar { output_polar_fn };

    output_polar << table_header << std::endl;
    size_t const tws_list_sz { tws_list->size() };

    for (size_t idx { 0u }; idx < tws_list_sz; ++idx) {
        float const tws { tws_list->at(idx) };
        TwsCurve_ptr polar_curve { polar->at(idx) };
        size_t const polar_curve_sz { polar_curve->size() };
        std::cout << tws;
        output_polar << tws;
        for (size_t jdx { 0u }; jdx < polar_curve_sz; ++jdx) {
            std::cout << "\t" << polar_curve->at(jdx)->first << "\t" << polar_curve->at(jdx)->second;
            output_polar << "\t" << polar_curve->at(jdx)->first << "\t" << polar_curve->at(jdx)->second;
        }
        std::cout << std::endl;
        output_polar << std::endl;
    }
}