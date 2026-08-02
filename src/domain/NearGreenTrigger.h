#pragma once

namespace opencaddie::domain {

class NearGreenTrigger {
public:
    NearGreenTrigger(double entryMetres = 35.0, double exitMetres = 50.0);

    void reset(int hole);
    [[nodiscard]] bool update(int hole, double distanceMetres, bool usableFix,
                              bool scored);
    [[nodiscard]] bool armed() const;

private:
    double m_entryMetres;
    double m_exitMetres;
    int m_hole = 0;
    bool m_armed = true;
};

} // namespace opencaddie::domain
