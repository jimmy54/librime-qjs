#ifndef FAKE_TRANSLATION_H_
#define FAKE_TRANSLATION_H_

#include <rime/candidate.h>
#include <rime/translation.h>

using namespace rime;

class FakeTranslation : public Translation {
public:
    bool Next() override {
        if (exhausted()) {
            return false;
        }
        if (++iter_ >= candidates_.size()) {
            set_exhausted(true);
        }
        return true;
    }

    an<Candidate> Peek() override {
        if (exhausted() || iter_ >= candidates_.size()) {
            set_exhausted(true);
            return nullptr;
        }
        return candidates_[iter_];
    }

    void Append(an<Candidate> candidate) {
        candidates_.push_back(candidate);
    }

private:
    std::vector<an<Candidate>> candidates_;
    size_t iter_ = 0;
};

#endif  // FAKE_TRANSLATION_H_
