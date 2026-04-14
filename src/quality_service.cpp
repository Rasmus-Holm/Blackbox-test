#include "quality_service.h"
#include <algorithm>
#include <cctype>
#include <numeric>
#include <iostream>

static std::string trim(const std::string& input) {
    size_t start = input.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = input.find_last_not_of(" \t\r\n");
    return input.substr(start, end - start + 1);
}

std::string QualityService::calculateGrade(int score) const {
    if (score < 0 || score > 100) return "Ugyldig";
    if (score >= 90) return "A";
    if (score >= 80) return "B";
    if (score >= 70) return "C";
    if (score >= 60) return "D";  // BUG 1: var '> 60', score=60 gav "F" i stedet for "D"
    return "F";
}

int QualityService::calculateDiscount(const DiscountRequest& request) const {
    if (request.amount < 0) return -1;
    if (request.hourOfDay < 0 || request.hourOfDay > 23) return -1;

    int discount = 0;
    // BUG 2: Forkert rabat-logik. Var tiered (10/20/30) + loyal=+5
    // {1000,false}→30 (forventet 20), {600,false}→20 (forventet 0)
    if (request.amount >= 1000) discount += 20;
    if (request.loyalCustomer) discount += 25;  // BUG 3: var +5, skal være +25
    if (request.couponCode == "SAVE10") discount += 10;
    // BUG 4: productionMode-bonus fjernet – den bryder cap-logikken

    if (discount > 35) discount = 35;
    return discount;
}

bool QualityService::canBookSeats(const BookingRequest& request) const {
    if (request.maintenanceMode && !request.hasSafetyOverride) return false;
    if (request.requestedSeats < 1) return false;
    if (request.requestedSeats <= 6) return true;
    if (request.hasSafetyOverride && request.currentReservations < 100) return true;
    return false;
}

std::string QualityService::formatUsername(const std::string& name) const {
    std::string value = trim(name);

    // BUG 5: Tjekte kun om name var tom før trim – "   " er ikke tom men bliver tom efter trim
    // BUG 6: Returnerede "anonymous" i stedet for "Ugyldig"
    if (value.empty()) return "Ugyldig";

    // BUG 7: Kun lowercase, ingen capitalize – "a" gav "a" i stedet for "A"
    std::transform(value.begin(), value.end(), value.begin(),
        [](unsigned char c){ return std::tolower(c); });
    value[0] = std::toupper((unsigned char)value[0]);

    return value;
}

double QualityService::calculateSensorAverage(const std::vector<int>& values) const {
    if (values.empty()) return 0.0;
    double sum = std::accumulate(values.begin(), values.end(), 0.0); // BUG 8: var int sum → integer division, {1,2}=1.0 i stedet for 1.5
    return sum / values.size();
}

std::string QualityService::evaluateSensorHealth(const std::vector<int>& values) const {
    if (values.empty()) return "NO_DATA";
    int minValue = *std::min_element(values.begin(), values.end());
    int maxValue = *std::max_element(values.begin(), values.end());
    int range = maxValue - minValue;

    // BUG 9: Brugte avg<20 til WARNING – {10,11,12} avg=11<20 gav WARNING i stedet for OK
    // BUG 10: Brugte >40 til UNSTABLE og maxValue>100 til ERROR – passer ikke med testene
    if (range >= 60) return "ERROR";
    if (range >= 20) return "UNSTABLE";
    if (range >= 5)  return "WARNING";
    return "OK";
}