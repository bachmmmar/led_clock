#pragma once


class String;

class Logger {

public:
    enum Severity {DEBUG=0, INFO=1, ERROR=2};
    explicit Logger();

    void setup(void);
    void log(Severity severity, const String & message);

    void setSeverity(Severity severity);

private:
    String getSeverityAsString(Severity severity) const;

    Severity current_severity_;
};

