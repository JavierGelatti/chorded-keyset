#include <ArduinoJson.h>
#include "json_utils.h"

String serializeJson(StaticJsonDocument<200> &documentToSerialize) {
  char* commandJson = (char*) malloc(200);
  serializeJson(documentToSerialize, commandJson, 200);
  auto result = String(commandJson);
  free(commandJson);
  return result;
}
