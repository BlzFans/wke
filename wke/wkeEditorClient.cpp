//////////////////////////////////////////////////////////////////////////


#include "wkeEditorClient.h"


//////////////////////////////////////////////////////////////////////////


namespace wke
{



static const unsigned CtrlKey = 1 << 0;
static const unsigned AltKey = 1 << 1;
static const unsigned ShiftKey = 1 << 2;

struct KeyDownEntry {
    unsigned virtualKey;
    unsigned modifiers;
    const char* name;
};

struct KeyPressEntry {
    unsigned charCode;
    unsigned modifiers;
    const char* name;
};

static const KeyDownEntry keyDownEntries[] = {
    { VK_LEFT,   0,                  "MoveLeft"                                    },
    { VK_LEFT,   ShiftKey,           "MoveLeftAndModifySelection"                  },
    { VK_LEFT,   CtrlKey,            "MoveWordLeft"                                },
    { VK_LEFT,   CtrlKey | ShiftKey, "MoveWordLeftAndModifySelection"              },
    { VK_RIGHT,  0,                  "MoveRight"                                   },
    { VK_RIGHT,  ShiftKey,           "MoveRightAndModifySelection"                 },
    { VK_RIGHT,  CtrlKey,            "MoveWordRight"                               },
    { VK_RIGHT,  CtrlKey | ShiftKey, "MoveWordRightAndModifySelection"             },
    { VK_UP,     0,                  "MoveUp"                                      },
    { VK_UP,     ShiftKey,           "MoveUpAndModifySelection"                    },
    { VK_PRIOR,  ShiftKey,           "MovePageUpAndModifySelection"                },
    { VK_DOWN,   0,                  "MoveDown"                                    },
    { VK_DOWN,   ShiftKey,           "MoveDownAndModifySelection"                  },
    { VK_NEXT,   ShiftKey,           "MovePageDownAndModifySelection"              },
    { VK_PRIOR,  0,                  "MovePageUp"                                  },
    { VK_NEXT,   0,                  "MovePageDown"                                },
    { VK_HOME,   0,                  "MoveToBeginningOfLine"                       },
    { VK_HOME,   ShiftKey,           "MoveToBeginningOfLineAndModifySelection"     },
    { VK_HOME,   CtrlKey,            "MoveToBeginningOfDocument"                   },
    { VK_HOME,   CtrlKey | ShiftKey, "MoveToBeginningOfDocumentAndModifySelection" },

    { VK_END,    0,                  "MoveToEndOfLine"                             },
    { VK_END,    ShiftKey,           "MoveToEndOfLineAndModifySelection"           },
    { VK_END,    CtrlKey,            "MoveToEndOfDocument"                         },
    { VK_END,    CtrlKey | ShiftKey, "MoveToEndOfDocumentAndModifySelection"       },

    { VK_BACK,   0,                  "DeleteBackward"                              },
    { VK_BACK,   ShiftKey,           "DeleteBackward"                              },
    { VK_DELETE, 0,                  "DeleteForward"                               },
    { VK_BACK,   CtrlKey,            "DeleteWordBackward"                          },
    { VK_DELETE, CtrlKey,            "DeleteWordForward"                           },

    { 'B',       CtrlKey,            "ToggleBold"                                  },
    { 'I',       CtrlKey,            "ToggleItalic"                                },

    { VK_ESCAPE, 0,                  "Cancel"                                      },
    { VK_OEM_PERIOD, CtrlKey,        "Cancel"                                      },
    { VK_TAB,    0,                  "InsertTab"                                   },
    { VK_TAB,    ShiftKey,           "InsertBacktab"                               },
    { VK_RETURN, 0,                  "InsertNewline"                               },
    { VK_RETURN, CtrlKey,            "InsertNewline"                               },
    { VK_RETURN, AltKey,             "InsertNewline"                               },
    { VK_RETURN, ShiftKey,           "InsertNewline"                               },
    { VK_RETURN, AltKey | ShiftKey,  "InsertNewline"                               },

    // It's not quite clear whether clipboard shortcuts and Undo/Redo should be handled
    // in the application or in WebKit. We chose WebKit.
    { 'C',       CtrlKey,            "Copy"                                        },
    { 'V',       CtrlKey,            "Paste"                                       },
    { 'X',       CtrlKey,            "Cut"                                         },
    { 'A',       CtrlKey,            "SelectAll"                                   },
    { VK_INSERT, CtrlKey,            "Copy"                                        },
    { VK_DELETE, ShiftKey,           "Cut"                                         },
    { VK_INSERT, ShiftKey,           "Paste"                                       },
    { 'Z',       CtrlKey,            "Undo"                                        },
    { 'Z',       CtrlKey | ShiftKey, "Redo"                                        },
};

static const KeyPressEntry keyPressEntries[] = {
    { '\t',   0,                  "InsertTab"                                   },
    { '\t',   ShiftKey,           "InsertBacktab"                               },
    { '\r',   0,                  "InsertNewline"                               },
    { '\r',   CtrlKey,            "InsertNewline"                               },
    { '\r',   AltKey,             "InsertNewline"                               },
    { '\r',   ShiftKey,           "InsertNewline"                               },
    { '\r',   AltKey | ShiftKey,  "InsertNewline"                               },
};

static const char* interpretKeyEvent(const WebCore::KeyboardEvent* evt)
{
    ASSERT(evt->type() == WebCore::eventNames().keydownEvent || evt->type() == WebCore::eventNames().keypressEvent);

    static HashMap<int, const char*>* keyDownCommandsMap = 0;
    static HashMap<int, const char*>* keyPressCommandsMap = 0;

    if (!keyDownCommandsMap) {
        keyDownCommandsMap = new HashMap<int, const char*>;
        keyPressCommandsMap = new HashMap<int, const char*>;

        for (unsigned i = 0; i < _countof(keyDownEntries); i++)
            keyDownCommandsMap->set(keyDownEntries[i].modifiers << 16 | keyDownEntries[i].virtualKey, keyDownEntries[i].name);

        for (unsigned i = 0; i < _countof(keyPressEntries); i++)
            keyPressCommandsMap->set(keyPressEntries[i].modifiers << 16 | keyPressEntries[i].charCode, keyPressEntries[i].name);
    }

    unsigned modifiers = 0;
    if (evt->shiftKey())
        modifiers |= ShiftKey;
    if (evt->altKey())
        modifiers |= AltKey;
    if (evt->ctrlKey())
        modifiers |= CtrlKey;

    if (evt->type() == WebCore::eventNames().keydownEvent) {
        int mapKey = modifiers << 16 | evt->keyCode();
        return mapKey ? keyDownCommandsMap->get(mapKey) : 0;
    }

    int mapKey = modifiers << 16 | evt->charCode();
    return mapKey ? keyPressCommandsMap->get(mapKey) : 0;
}




EditorClient::~EditorClient()
{

}
void EditorClient::pageDestroyed()
{
    delete this;
}

WebCore::TextCheckerClient* EditorClient::textChecker()
{
    return this;
}

void EditorClient::requestCheckingOfString(WebCore::SpellChecker*, int, WebCore::TextCheckingTypeMask, const WTF::String&)
{

}

void EditorClient::getGuessesForWord(const WTF::String& word, const WTF::String& context, WTF::Vector<WTF::String>& guesses)
{

}

void EditorClient::setInputMethodState(bool enabled)
{

}

void EditorClient::willSetInputMethodState()
{

}

bool EditorClient::spellingUIIsShowing()
{
    return false;
}

void EditorClient::showSpellingUI(bool show)
{

}

void EditorClient::updateSpellingUIWithMisspelledWord(const String&)
{

}

void EditorClient::updateSpellingUIWithGrammarString(const String&, const WebCore::GrammarDetail& detail)
{

}

void EditorClient::checkGrammarOfString(const UChar*, int length, Vector<WebCore::GrammarDetail>&, int* badGrammarLocation, int* badGrammarLength)
{

}

String EditorClient::getAutoCorrectSuggestionForMisspelledWord(const String& misspelledWord)
{
    return String();
}

void EditorClient::checkSpellingOfString(const UChar*, int length, int* misspellingLocation, int* misspellingLength)
{

}

void EditorClient::learnWord(const String&)
{

}

void EditorClient::ignoreWordInSpellDocument(const String&)
{

}

void EditorClient::textDidChangeInTextArea(WebCore::Element*)
{

}

void EditorClient::textWillBeDeletedInTextField(WebCore::Element*)
{

}

bool EditorClient::doTextFieldCommandFromEvent(WebCore::Element*, WebCore::KeyboardEvent*)
{
    return false;
}

void EditorClient::textDidChangeInTextField(WebCore::Element*)
{

}

void EditorClient::textFieldDidEndEditing(WebCore::Element*)
{

}

void EditorClient::textFieldDidBeginEditing(WebCore::Element*)
{

}

void EditorClient::handleInputMethodKeydown(WebCore::KeyboardEvent*)
{

}

void EditorClient::handleKeyboardEvent(WebCore::KeyboardEvent* evt)
{
    WebCore::Node* node = evt->target()->toNode();
    ASSERT(node);
    WebCore::Frame* frame = node->document()->frame();
    ASSERT(frame);

    const WebCore::PlatformKeyboardEvent* keyEvent = evt->keyEvent();
    if (!keyEvent || keyEvent->isSystemKey())  // do not treat this as text input if it's a system key event
        return;

    WebCore::Editor::Command command = frame->editor()->command(interpretKeyEvent(evt));

    if (keyEvent->type() == WebCore::PlatformKeyboardEvent::RawKeyDown) {
        // WebKit doesn't have enough information about mode to decide how commands that just insert text if executed via Editor should be treated,
        // so we leave it upon WebCore to either handle them immediately (e.g. Tab that changes focus) or let a keypress event be generated
        // (e.g. Tab that inserts a Tab character, or Enter).
        if (!command.isTextInsertion() && command.execute(evt))
            evt->setDefaultHandled();
        return;
    }

    if (command.execute(evt))
    {
        evt->setDefaultHandled();
        return;
    }

    // Don't insert null or control characters as they can result in unexpected behaviour
    if (evt->charCode() < ' ')
        return;

    if (frame->editor()->insertText(evt->keyEvent()->text(), evt))
        evt->setDefaultHandled();
}

void EditorClient::redo()
{

}

void EditorClient::undo()
{

}

bool EditorClient::canRedo() const 
{
    return false;
}

bool EditorClient::canUndo() const 
{
    return false;
}

bool EditorClient::canPaste(WebCore::Frame*, bool defaultValue) const 
{
    return defaultValue;
}

bool EditorClient::canCopyCut(WebCore::Frame*, bool defaultValue) const 
{
    return defaultValue;
}

void EditorClient::clearUndoRedoOperations()
{

}

void EditorClient::registerCommandForRedo(PassRefPtr<WebCore::EditCommand>)
{

}

void EditorClient::registerCommandForUndo(PassRefPtr<WebCore::EditCommand>)
{

}

void EditorClient::didSetSelectionTypesForPasteboard()
{

}

void EditorClient::didWriteSelectionToPasteboard()
{

}

void EditorClient::didEndEditing()
{

}

void EditorClient::respondToChangedSelection()
{

}

void EditorClient::respondToChangedContents()
{

}

void EditorClient::didBeginEditing()
{

}

bool EditorClient::shouldMoveRangeAfterDelete(WebCore::Range*, WebCore::Range*)
{
    return true;
}

bool EditorClient::shouldApplyStyle(WebCore::CSSStyleDeclaration*, WebCore::Range*)
{
    return true;
}

bool EditorClient::shouldChangeSelectedRange(WebCore::Range* fromRange, WebCore::Range* toRange, WebCore::EAffinity, bool stillSelecting)
{
    return true;
}

bool EditorClient::shouldInsertText(const String&, WebCore::Range*, WebCore::EditorInsertAction)
{
    return true;
}

bool EditorClient::shouldInsertNode(WebCore::Node*, WebCore::Range*, WebCore::EditorInsertAction)
{
    return true;
}

bool EditorClient::shouldEndEditing(WebCore::Range*)
{
    return true;
}

bool EditorClient::shouldBeginEditing(WebCore::Range*)
{
    return true;
}

int EditorClient::spellCheckerDocumentTag()
{
    return 0;
}

void EditorClient::toggleGrammarChecking()
{

}

bool EditorClient::isGrammarCheckingEnabled()
{
    return false;
}

void EditorClient::toggleContinuousSpellChecking()
{

}

bool EditorClient::isContinuousSpellCheckingEnabled()
{
    return false;
}

bool EditorClient::isSelectTrailingWhitespaceEnabled()
{
    return false;
}

bool EditorClient::smartInsertDeleteEnabled()
{
    return false;
}

bool EditorClient::shouldShowDeleteInterface(WebCore::HTMLElement*)
{
    return false;
}

bool EditorClient::shouldDeleteRange(WebCore::Range*)
{
    return true;
}




};//namespace wke