#include "PolarFile.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstdint>
#include <cctype>
#include <charconv>
#include <cassert>
#include <string>
#include <string_view>
#include <vector>
#include <deque>
#include <map>
#include <utility>
#include <algorithm>

constexpr float version { 0.5f };

using namespace std::literals::string_literals;
using namespace std::literals::string_view_literals;

void display_help(std::string const& name_);

using ArgStrings = std::vector<std::string>;
ArgStrings process_arguments(int argc_, char* const argv_[]);

int main(int argc, char* const argv[]) {

    if (argc < 4) {
        display_help(argv[0]);
        std::exit(666);
    }
    std::cout << "pqPolarConverter" << std::endl;
    std::cout << "Version: " << version << std::endl;
    ArgStrings const args { process_arguments(argc, argv) };

    std::string const command { args.at(1) };
    std::string const input_filename { args.at(2) };
    std::string const output_filename { args.at(3) };

    std::cout << "Inputfile => " << input_filename << " Outpuffile => " << output_filename << std::endl;

    Sailing::PolarFiles::PolarFile new_polar;
    if (command == "convert") {
        new_polar.import_polar(input_filename);
    } else if (command == "clean") {
        new_polar.read_polar(input_filename);
    }
    new_polar.normalise_polar();
    new_polar.export_polar(output_filename);
}

void display_help(std::string const& name_) {
    std::cout << name_ << " => Polar File Conversion for Expedition"s << std::endl << std::endl;
    std::cout << "Usage: "s << name_ << " <command> <input file> <outputfile>"s << std::endl;
    std::cout << "Commands: clean or convert" << std::endl;
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

Sailing::PolarFiles::StrVec Sailing::PolarFiles::PolarFile::break_strings(Sailing::PolarFiles::str_ptr input, char const cut_char) {
    StrVec results {};
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

Sailing::PolarFiles::FloatVec Sailing::PolarFiles::PolarFile::break_strings_float(Sailing::PolarFiles::str_ptr input, char const cut_char) {
    FloatVec results {};
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

    std::cout << "Importing non-Expedition Polar: " << input_polar_fn << std::endl;

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
    *tws_list = break_strings_float(tline, cut_char);

    std::shared_ptr<OldPolar> old_polar_data { std::make_shared<OldPolar>() };

    for (std::shared_ptr<std::string> line { std::make_shared<std::string>() }; std::getline(*polar_file, *line);) {
        std::cout << *line << std::endl;

        std::shared_ptr<std::vector<float>> row { std::make_shared<std::vector<float>>(break_strings_float(line, cut_char)) };

        float const twa { row->at(0) };
        old_polar_data->twa_list->push_back(twa);

        old_polar_data->old_polar_map->emplace(twa, row);
    }

    //Put in onject...
    polar = std::make_shared<std::vector<TwsCurve_ptr>>();
    size_t const tws_list_sz { tws_list->size() };
    size_t const twa_list_sz { old_polar_data->twa_list->size() };
    for (size_t idx { 0u }; idx < tws_list_sz; ++idx) {
        float tws { tws_list->at(idx) };
        std::vector<float> new_row {};
        for (size_t jdx { 0u }; jdx < twa_list_sz; ++jdx) {
            float key { old_polar_data->twa_list->at(jdx) };
            float val { old_polar_data->old_polar_map->at(key)->at(idx + 1) };
            new_row.push_back(val);
        }
        TwsCurve_ptr polar_curves { build_pairs(*old_polar_data->twa_list, new_row) };

        polar->push_back(polar_curves);
    }

    return;
}

void Sailing::PolarFiles::PolarFile::export_polar(std::string const& output_polar_fn) {
    std::shared_ptr<std::ofstream> output_polar { std::make_shared<std::ofstream>(output_polar_fn) };

    *output_polar << table_header << std::endl;
    for (auto comment : *comments) {
        *output_polar << *comment << std::endl;
    }
    size_t const tws_list_sz { tws_list->size() };

    for (size_t idx { 0u }; idx < tws_list_sz; ++idx) {
        float const tws { tws_list->at(idx) };
        TwsCurve_ptr polar_curve { polar->at(idx) };
        size_t const polar_curve_sz { polar_curve->size() };
        std::cout << tws;
        *output_polar << tws;
        for (size_t jdx { 0u }; jdx < polar_curve_sz; ++jdx) {
            std::cout << "\t" << polar_curve->at(jdx)->first << "\t" << polar_curve->at(jdx)->second;
            *output_polar << "\t" << polar_curve->at(jdx)->first << "\t" << polar_curve->at(jdx)->second;
        }
        std::cout << std::endl;
        *output_polar << std::endl;
    }
}

void Sailing::PolarFiles::PolarFile::read_polar(std::string const& input_polar_fn) {
    std::shared_ptr<std::ifstream> polar_file { std::make_shared<std::ifstream>(input_polar_fn) };

    std::cout << "Reading Expedition Polar: " << input_polar_fn << std::endl;

    if (polar_file->is_open() == false) {
        std::cerr << "ERROR: Unable to open file  " << input_polar_fn << std::endl;
        return;
    }

    char cut_char {};

    for (std::shared_ptr<std::string> line { std::make_shared<std::string>() }; std::getline(*polar_file, *line);) {
        std::cout << *line << std::endl;

        if (line->at(0) == '!') {
            if (*line != table_header) {
                comments->push_back(line);
            }
            continue;
        }
        // Find fisrt non-number char -- delimiter.
        size_t eon {};
        for (size_t idx {0}; cut_char == '\0' && idx < line->size(); ++idx) {
            if (std::isdigit(line->at(idx)) == 0) { //Any charecter not a digit must be the cut char...
                cut_char = line->at(idx);
                eon = idx;
                break;
            }
        }

        FloatVec broken_line { break_strings_float(line, cut_char) };
        // Broken line must have odd number for tws_ and pairs of values
        size_t const broken_line_sz { broken_line.size() };
        assert(broken_line.size() % 2 == 1);

        float const tws_ { broken_line.at(0) };
        TwsCurve_ptr tws_curve { std::make_shared<TwsCurve>() };
        for (size_t idx { 1 }; idx < broken_line.size(); idx += 2) {
            FloatPair_ptr fp { std::make_shared<FloatPair>(broken_line.at(idx), broken_line.at(idx + 1)) };
            tws_curve->push_back(fp);
        }

        tws_list->push_back(tws_);
        polar->push_back(tws_curve);
    }
}

void Sailing::PolarFiles::PolarFile::normalise_polar() {
    std::cout << "Cleaning polar file." << std::endl;

    std::for_each(std::begin(*polar), std::end(*polar), [](auto& tws_curve) {
        std::sort(std::begin(*tws_curve), std::end(*tws_curve), [](auto& p0, auto& p1) {
            return p0->first < p1->first;
        });
    });

    //Clean duplicates...
    for (auto tws_curve : *polar) {
        size_t tws_curve_sz { tws_curve->size() };
        for (auto tws_curve_iter { std::begin(*tws_curve) }; tws_curve_iter + 1 != std::end(*tws_curve); ++tws_curve_iter) {
            FloatPair_ptr const &p0 { *tws_curve_iter };
            FloatPair_ptr const &p1 { *(tws_curve_iter + 1) };
            if (p0->first == p1->first) {
                std::cout << "Found conflicting entries in in TWS Curve..." << std::endl;
                if (p0->second == p1->second) {
                    std::cout << "Entry is identical - dropping duplicate." << std::endl;
                    tws_curve->erase(tws_curve_iter + 1);
                } else { //User intervention required...
                    //A/B choice of which entry to drop
                    std::cout << "A: " << polar_entry_txt(p0) << std::endl;
                    std::cout << "B: " << polar_entry_txt(p1) << std::endl;
                    bool valid { false };
                    do {
                        std::cout << "Please select entry to keep: ";
                        char input {};
                        std::cin >> input;
                        switch (std::tolower(input)) {
                        case 'a':
                            tws_curve_iter = tws_curve->erase(tws_curve_iter);
                            tws_curve_iter -= 1;
                            valid = true;
                            break;
                        case 'b':
                            tws_curve->erase(tws_curve_iter + 1);
                            valid = true;
                            break;
                        default:
                            valid = false;
                        }
                    } while (!valid);
                }
            }
        }
    }
}

std::string Sailing::PolarFiles::PolarFile::polar_entry_txt(FloatPair_ptr fp) {
    std::stringstream ret;
    ret << "TWA: " << fp->first << " Parameter: " << fp->second;

    return ret.str();
}