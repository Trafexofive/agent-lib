#pragma once // Prevent multiple inclusions

#include <algorithm>  // For std::sort, std::find_if
#include <chrono>     // For generating basic IDs
#include <filesystem> // Requires C++17
#include <fstream>
#include <iomanip> // For formatting time in IDs
#include <iostream>
#include <mutex> // For protecting file access
#include <sstream>
#include <string>
#include <vector>

#include "json/json.h"

// --- Data Structure for Events ---
struct CalendarEvent {
  std::string id;   // Unique identifier for the event
  std::string date; // Format: YYYY-MM-DD
  std::string time; // Format: HH:MM (or empty for all-day)
  std::string description;

  bool operator<(const CalendarEvent &other) const {
    if (date != other.date) return date < other.date;
    if (time.empty() && !other.time.empty()) return true;
    if (!time.empty() && other.time.empty()) return false;
    return time < other.time;
  }
};

// --- Globals ---
const std::string CALENDAR_DATA_FILE = ".calendar_data.json";
std::mutex calendarMutex;

// --- Helper Functions ---
std::string generateEventId() {
  auto now = std::chrono::system_clock::now();
  auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
  auto epoch = now_ms.time_since_epoch();
  long long timestamp_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count();
  static int counter = 0;
  std::stringstream ss;
  ss << timestamp_ms << "-" << (counter++);
  return ss.str();
}

std::vector<CalendarEvent> loadEventsFromFile() {
  std::vector<CalendarEvent> events;
  std::ifstream file(CALENDAR_DATA_FILE);
  if (!file.is_open()) {
    if (std::filesystem::exists(CALENDAR_DATA_FILE)) {
      std::cerr << "[WARN] calendarTool: Could not open '" << CALENDAR_DATA_FILE << "' for reading." << std::endl;
    } else {
      // std::cout << "[INFO] calendarTool: Data file '" << CALENDAR_DATA_FILE << "' not found. Starting fresh." << std::endl; // Less noisy
    }
    return events;
  }

  Json::Value root;
  Json::Reader reader;
  if (!reader.parse(file, root)) {
    std::cerr << "[ERROR] calendarTool: Failed to parse JSON from '" << CALENDAR_DATA_FILE << "'. Error: " << reader.getFormattedErrorMessages() << std::endl;
    file.close();
    return events;
  }
  file.close();

  if (!root.isArray()) {
    std::cerr << "[ERROR] calendarTool: JSON root in '" << CALENDAR_DATA_FILE << "' is not an array." << std::endl;
    return events;
  }

  for (const auto &item : root) {
    try {
      CalendarEvent event;
      event.id = item.get("id", generateEventId()).asString();
      event.date = item["date"].asString();
      event.time = item.get("time", "").asString();
      event.description = item["description"].asString();
      events.push_back(event);
    } catch (const Json::Exception &e) {
      std::cerr << "[WARN] calendarTool: Skipping invalid event item in JSON. Error: " << e.what() << std::endl;
    } catch (const std::exception &e) {
      std::cerr << "[WARN] calendarTool: Skipping event due to std exception: " << e.what() << std::endl;
    }
  }
  std::sort(events.begin(), events.end());
  return events;
}

bool saveEventsToFile(const std::vector<CalendarEvent> &events) {
  Json::Value root(Json::arrayValue);
  for (const auto &event : events) {
    Json::Value item;
    item["id"] = event.id;
    item["date"] = event.date;
    if (!event.time.empty()) item["time"] = event.time;
    item["description"] = event.description;
    root.append(item);
  }

  std::ofstream file(CALENDAR_DATA_FILE, std::ios::trunc);
  if (!file.is_open()) {
    std::cerr << "[ERROR] calendarTool: Could not open '" << CALENDAR_DATA_FILE << "' for writing." << std::endl;
    return false;
  }
  Json::FastWriter writer;
  file << writer.write(root);
  if (file.fail() || file.bad()) {
    std::cerr << "[ERROR] calendarTool: Failed writing events to '" << CALENDAR_DATA_FILE << "'." << std::endl;
    file.close();
    return false;
  }
  file.close();
  return true;
}

bool isValidDate(const std::string &date) {
  if (date.length() != 10) return false;
  return date[4] == '-' && date[7] == '-' && isdigit(date[0]) &&
         isdigit(date[1]) && isdigit(date[2]) && isdigit(date[3]) &&
         isdigit(date[5]) && isdigit(date[6]) && isdigit(date[8]) &&
         isdigit(date[9]);
}

bool isValidTime(const std::string &time) {
  if (time.empty()) return true;
  if (time.length() != 5) return false;
  return time[2] == ':' && isdigit(time[0]) && isdigit(time[1]) &&
         isdigit(time[3]) && isdigit(time[4]);
}

// --- Tool Implementation (Returns JSON String) ---
std::string calendarTool(const Json::Value &params) {
  Json::Value response;
  std::lock_guard<std::mutex> lock(calendarMutex);

  if (params == Json::nullValue || !params.isObject()) {
    response["status"] = "error";
    response["result"] = "Error: calendarTool requires a JSON object with parameters.";
    return response.toStyledString();
  }
  if (!params.isMember("action") || !params["action"].isString()) {
    response["status"] = "error";
    response["result"] = "Error: Missing or invalid required string parameter 'action' ('add' or 'list').";
    return response.toStyledString();
  }

  std::string action = params["action"].asString();
  std::vector<CalendarEvent> events = loadEventsFromFile();

  if (action == "add") {
    if (!params.isMember("date") || !params["date"].isString()) {
      response["status"] = "error";
      response["result"] = "Error [add action]: Missing or invalid required string parameter 'date' (YYYY-MM-DD).";
      return response.toStyledString();
    }
    if (!params.isMember("description") || !params["description"].isString() || params["description"].asString().empty()) {
      response["status"] = "error";
      response["result"] = "Error [add action]: Missing or invalid non-empty string parameter 'description'.";
      return response.toStyledString();
    }

    CalendarEvent newEvent;
    newEvent.date = params["date"].asString();
    newEvent.description = params["description"].asString();
    newEvent.time = params.get("time", "").asString();

    if (!isValidDate(newEvent.date)) {
      response["status"] = "error";
      response["result"] = "Error [add action]: Invalid date format. Expected YYYY-MM-DD. Received: '" + newEvent.date + "'";
      return response.toStyledString();
    }
    if (!isValidTime(newEvent.time)) {
      response["status"] = "error";
      response["result"] = "Error [add action]: Invalid time format. Expected HH:MM or empty. Received: '" + newEvent.time + "'";
      return response.toStyledString();
    }

    newEvent.id = generateEventId();
    events.push_back(newEvent);
    std::sort(events.begin(), events.end());

    if (saveEventsToFile(events)) {
      std::stringstream successMsg;
      successMsg << "Event added with ID '" << newEvent.id << "'.
";
      successMsg << "Date: " << newEvent.date;
      if (!newEvent.time.empty()) successMsg << " Time: " << newEvent.time;
      successMsg << "
Description: " << newEvent.description;
      response["status"] = "success";
      response["result"] = successMsg.str();
      return response.toStyledString();
    } else {
      response["status"] = "error";
      response["result"] = "Error [add action]: Failed to save event to file.";
      return response.toStyledString();
    }

  } else if (action == "list") {
    std::string filterDate = params.get("date", "").asString();
    if (!filterDate.empty() && !isValidDate(filterDate)) {
      response["status"] = "error";
      response["result"] = "Error [list action]: Invalid date format for filtering. Expected YYYY-MM-DD. Received: '" + filterDate + "'";
      return response.toStyledString();
    }

    std::stringstream output;
    int count = 0;
    if (filterDate.empty()) {
      output << "All Events:
";
    } else {
      output << "Events for date " << filterDate << ":
";
    }

    for (const auto &event : events) {
      bool match = filterDate.empty() || (event.date == filterDate);
      if (match) {
        count++;
        output << "- ID: " << event.id << " | Date: " << event.date;
        if (!event.time.empty()) output << " Time: " << event.time;
        output << " | Desc: " << event.description << "
";
      }
    }

    if (count == 0) {
      output << (filterDate.empty() ? "(No events scheduled)
" : "(No events found for this date)
");
    }

    std::string result = output.str();
    if (!result.empty() && result.back() == '
') result.pop_back(); // Trim trailing newline

    response["status"] = "success";
    response["result"] = result;
    return response.toStyledString();

  } else {
    response["status"] = "error";
    response["result"] = "Error: Unknown action '" + action + "'. Supported actions: 'add', 'list'.";
    return response.toStyledString();
  }
}
