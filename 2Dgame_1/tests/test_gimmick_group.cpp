#include "../Stage/GimmickGroup.h"

#include <cassert>
#include <stdexcept>
#include <string>

struct TestBlock {
    int id = 0;
    bool triggered = false;
};

struct TestComment {
    std::string message;
    bool triggered = false;
};

int main() {
    GimmickGroup<TestBlock, 2> blocks;
    assert(blocks.Size() == 0);
    assert(blocks.Add({1, false}));
    assert(blocks.Add({2, false}));
    assert(!blocks.Add({3, false})); // 容量超過時は追加しない

    blocks.Get(0).triggered = true;
    assert(blocks.Get(0).triggered);
    blocks.Reset();
    assert(!blocks.Get(0).triggered);

    blocks.Get(0).triggered = true;
    blocks.SaveInitialState();
    blocks.Get(0).triggered = false;
    blocks.Reset();
    assert(blocks.Get(0).triggered);

    const auto& readOnly = blocks;
    assert(readOnly.Get(1).id == 2);

    bool outOfRange = false;
    try {
        blocks.Get(2);
    } catch (const std::out_of_range&) {
        outOfRange = true;
    }
    assert(outOfRange);

    blocks.Clear();
    assert(blocks.Size() == 0);
    assert(blocks.Add({4, false}));
    assert(blocks.Get(0).id == 4);

    // CommentBlockのようにstd::stringを含む型も保存・復元できる。
    GimmickGroup<TestComment, 2> comments;
    assert(comments.Add({"こんにちは", false}));
    comments.Get(0).message = "変更後";
    comments.Get(0).triggered = true;
    comments.Reset();
    assert(comments.Get(0).message == "こんにちは");
    assert(!comments.Get(0).triggered);
}
