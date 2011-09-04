
#include <WebCore/EditorClient.h>
#include <WebCore/EditCommand.h>
#include <WebCore/KeyboardEvent.h>
#include <WebCore/EventNames.h>
#include <WebCore/TextCheckerClient.h>

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

    class EditorClient : public WebCore::EditorClient, public WebCore::TextCheckerClient
    {
    public:
        virtual ~EditorClient() {  }

        virtual void pageDestroyed() override
        {
            delete this;
        }

        virtual bool shouldDeleteRange(WebCore::Range*) override
        {
            return true;
        }

        virtual bool shouldShowDeleteInterface(WebCore::HTMLElement*) override
        {
            return false;
        }

        virtual bool smartInsertDeleteEnabled() override
        {
            return false;
        }

        virtual bool isSelectTrailingWhitespaceEnabled() override
        {
            return false;
        }

        virtual bool isContinuousSpellCheckingEnabled() override
        {
            return false;
        }

        virtual void toggleContinuousSpellChecking() override
        {
        }

        virtual bool isGrammarCheckingEnabled() override
        {
            return false;
        }

        virtual void toggleGrammarChecking() override
        {
        }

        virtual int spellCheckerDocumentTag() override
        {
            return 0;
        }

        virtual bool shouldBeginEditing(WebCore::Range*) override
        {
            return true;
        }

        virtual bool shouldEndEditing(WebCore::Range*) override
        {
            return true;
        }

        virtual bool shouldInsertNode(WebCore::Node*, WebCore::Range*, WebCore::EditorInsertAction) override
        {
            return true;
        }

        virtual bool shouldInsertText(const String&, WebCore::Range*, WebCore::EditorInsertAction) override
        {
            return true;
        }

        virtual bool shouldChangeSelectedRange(WebCore::Range* fromRange, WebCore::Range* toRange, WebCore::EAffinity, bool stillSelecting) override
        {
            return true;
        }

        virtual bool shouldApplyStyle(WebCore::CSSStyleDeclaration*, WebCore::Range*) override
        {
            return true;
        }

        virtual bool shouldMoveRangeAfterDelete(WebCore::Range*, WebCore::Range*) override
        {
            return true;
        }

        virtual void didBeginEditing() override
        {
        }

        virtual void respondToChangedContents() override
        {
        }

        virtual void respondToChangedSelection() override
        {
        }

        virtual void didEndEditing() override
        {
        }

        virtual void didWriteSelectionToPasteboard() override
        {
        }

        virtual void didSetSelectionTypesForPasteboard() override
        {
        }

        virtual void registerCommandForUndo(PassRefPtr<WebCore::EditCommand>) override
        {
        }

        virtual void registerCommandForRedo(PassRefPtr<WebCore::EditCommand>) override
        {
        }

        virtual void clearUndoRedoOperations() override
        {
        }

        virtual bool canCopyCut(WebCore::Frame*, bool defaultValue) const override
        {
            return defaultValue;
        }

        virtual bool canPaste(WebCore::Frame*, bool defaultValue) const override
        {
            return defaultValue;
        }

        virtual bool canUndo() const override
        {
            return false;
        }

        virtual bool canRedo() const override
        {
            return false;
        }

        virtual void undo() override
        {
        }

        virtual void redo() override
        {
        }

        virtual void handleKeyboardEvent(WebCore::KeyboardEvent* evt) override
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

        virtual void handleInputMethodKeydown(WebCore::KeyboardEvent*) override
        {
        }

        virtual void textFieldDidBeginEditing(WebCore::Element*) override
        {
        }

        virtual void textFieldDidEndEditing(WebCore::Element*) override
        {
        }

        virtual void textDidChangeInTextField(WebCore::Element*) override
        {
        }

        virtual bool doTextFieldCommandFromEvent(WebCore::Element*, WebCore::KeyboardEvent*) override
        {
            return false;
        }

        virtual void textWillBeDeletedInTextField(WebCore::Element*) override
        {
        }

        virtual void textDidChangeInTextArea(WebCore::Element*) override
        {
        }

        virtual void ignoreWordInSpellDocument(const String&) override
        {
        }

        virtual void learnWord(const String&) override
        {
        }

        virtual void checkSpellingOfString(const UChar*, int length, int* misspellingLocation, int* misspellingLength) override
        {
        }

        virtual String getAutoCorrectSuggestionForMisspelledWord(const String& misspelledWord) override
        {
            return String();
        }

        virtual void checkGrammarOfString(const UChar*, int length, Vector<WebCore::GrammarDetail>&, int* badGrammarLocation, int* badGrammarLength) override
        {
        }

        virtual void updateSpellingUIWithGrammarString(const String&, const WebCore::GrammarDetail& detail) override
        {
        }

        virtual void updateSpellingUIWithMisspelledWord(const String&) override
        {
        }

        virtual void showSpellingUI(bool show) override
        {
        }

        virtual bool spellingUIIsShowing() override
        {
            return false;
        }

        virtual void willSetInputMethodState() override
        {
        }

        virtual void setInputMethodState(bool enabled) override
        {
        }

        virtual void getGuessesForWord(const WTF::String& word, const WTF::String& context, WTF::Vector<WTF::String>& guesses) override
        {
        }

        virtual void requestCheckingOfString(WebCore::SpellChecker*, int, WebCore::TextCheckingTypeMask, const WTF::String&) override
        {
        }

        virtual WebCore::TextCheckerClient* textChecker() override { return this; }
    };
}