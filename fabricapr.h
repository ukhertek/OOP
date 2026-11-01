#pragma once
#include <istream>
#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

inline constexpr int kEof = std::char_traits<char>::eof();

inline std::string trim(const std::string& s) {
    const char* spaces = " \t\r\n\f\v";
    const auto b = s.find_first_not_of(spaces);
    if (b == std::string::npos) return {};
    const auto e = s.find_last_not_of(spaces);
    return s.substr(b, e - b + 1);
}

// Иерархия предложений

class Sentence {
public:
    explicit Sentence(const std::string& text) : text_(trim(text)) {}
    virtual ~Sentence() = default;

    const std::string& text() const { return text_; }
    virtual const char* kind() const = 0;   // вид предложения
    virtual const char* sign() const = 0;   // завершающий знак

    void print(std::ostream& os) const {
        os << '[' << kind() << "] " << text_ << sign();
    }

private:
    std::string text_;
};

inline std::ostream& operator<<(std::ostream& os, const Sentence& s) {
    s.print(os);
    return os;
}

class DeclarativeSentence : public Sentence {     //повеств предл заканч-ая точкой .
public:
    using Sentence::Sentence;
    const char* kind() const override { return "Повествовательное"; }
    const char* sign() const override { return "."; }
};

class InterrogativeSentence : public Sentence {   //вопросит предл заканч-ая ?
public:
    using Sentence::Sentence;
    const char* kind() const override {
        return "Вопросительное";
    }
    const char* sign() const override {
        return "?";
    }
};

class ExclamatorySentence : public Sentence {     //восклиц предл заканч-ая !
public:
    using Sentence::Sentence;
    const char* kind() const override {
        return "Восклицательное";
    }
    const char* sign() const override {
        return "!";
    }
};

class SemicolonSentence : public Sentence {       //предл заканч-ая с точкой с запятой ;
public:
    using Sentence::Sentence;
    const char* kind() const override {
        return "С точкой с запятой";
    }
    const char* sign() const override {
        return ";";
    }
};


class UnterminatedSentence : public Sentence {    //предл без знака EOF
public:
    using Sentence::Sentence;
    const char* kind() const override {
        return "Без знака (EOF)";
    }
    const char* sign() const override {
        return "";
    }
};

//Иерархия парсеров

class SentenceParser {
public:
    virtual ~SentenceParser() = default;
    std::unique_ptr<Sentence> parse(const std::string& text, int sign) const {
        if (!recognizes(sign))
            return nullptr;
        return build(text);
    }

private:
    virtual bool recognizes(int sign) const = 0;
    virtual std::unique_ptr<Sentence> build(const std::string& text) const = 0;
};

// Парсер, который узнаёт один конкретный символ
class CharParser : public SentenceParser {
public:
    explicit CharParser(char sign) : sign_(sign) {}

private:
    bool recognizes(int sign) const override {
        return sign == static_cast<unsigned char>(sign_);
    }
    char sign_;
};

class DotParser : public CharParser {
public:
    DotParser() : CharParser('.') {}
private:
    std::unique_ptr<Sentence> build(const std::string& text) const override {
        return std::make_unique<DeclarativeSentence>(text);
    }
};

class QuestionParser : public CharParser {
public:
    QuestionParser() : CharParser('?') {}
private:
    std::unique_ptr<Sentence> build(const std::string& text) const override {
        return std::make_unique<InterrogativeSentence>(text);
    }
};

class ExclamationParser : public CharParser {
public:
    ExclamationParser() : CharParser('!') {}
private:
    std::unique_ptr<Sentence> build(const std::string& text) const override {
        return std::make_unique<ExclamatorySentence>(text);
    }
};

class SemicolonParser : public CharParser {
public:
    SemicolonParser() : CharParser(';') {}
private:
    std::unique_ptr<Sentence> build(const std::string& text) const override {
        return std::make_unique<SemicolonSentence>(text);
    }
};

class EofParser : public SentenceParser {
private:
    bool recognizes(int sign) const override { return sign == kEof; }
    std::unique_ptr<Sentence> build(const std::string& text) const override {
        return std::make_unique<UnterminatedSentence>(text);
    }
};

//Фабрика предложений

class SentenceFactory {
public:
    SentenceFactory() {
        parsers_.push_back(std::make_unique<DotParser>());
        parsers_.push_back(std::make_unique<QuestionParser>());
        parsers_.push_back(std::make_unique<ExclamationParser>());
        parsers_.push_back(std::make_unique<SemicolonParser>());
        parsers_.push_back(std::make_unique<EofParser>());
    }

    // Предлагает знак всем парсерам; предложение строит тот, кто его узнал
    std::unique_ptr<Sentence> create(const std::string& text, int sign) const {
        for (const auto& p : parsers_)
            if (auto s = p->parse(text, sign))
                return s;
        return nullptr;
    }

private:
    std::vector<std::unique_ptr<SentenceParser>> parsers_;
};

// Чтение потока в вектор предложений

inline std::vector<std::unique_ptr<Sentence>>
readSentences(std::istream& in, const SentenceFactory& factory) {
    std::vector<std::unique_ptr<Sentence>> sentences;
    std::string text;

    for (;;) {
        const int c = in.get();                  // в конце файла вернёт kEof

        if (auto s = factory.create(text, c)) {  // какой-то парсер узнал знак c
            if (!s->text().empty())              // пустые ("...", "?!") пропускаем
                sentences.push_back(std::move(s));
            text.clear();
        } else  {
            text += static_cast<char>(c);        // обычный символ внутри предложения
        }

        if (c == kEof)
            break;
    }
    return sentences;
}
