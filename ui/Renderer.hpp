#pragma once
#include <vector>
#include <string>

namespace ui {

enum class SearchMode  { EXACT, FUZZY, ABBREVIATION };
enum class AppMode { SEARCH, MENU, INPUT };

struct RenderState {
 
    std::string query;
    std::vector<std::string> suggestions;
    int selected_index = 0;
    SearchMode search_mode = SearchMode::EXACT;
    bool cache_hit = false;
    double elapsed_ms = 0.0;

  
    AppMode app_mode = AppMode::SEARCH;
   
    std::string input_prompt;   
    std::string input_buffer;   
    std::string status_msg;    
};

class Renderer {
public:
    static void draw(const RenderState& state);

private:
    static void clear_screen();
    static void draw_header();
    static void draw_search(const RenderState& s);
    static void draw_menu  (const RenderState& s);
    static void draw_input (const RenderState& s);
    static const char* mode_label(SearchMode m);
};

} 
