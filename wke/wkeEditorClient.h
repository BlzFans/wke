#ifndef WKE_EDITOR_CLIENT_H
#define WKE_EDITOR_CLIENT_H

//////////////////////////////////////////////////////////////////////////


#include <WebCore/config.h>
#include <WebCore/EditorClient.h>
#include <WebCore/EditCommand.h>
#include <WebCore/KeyboardEvent.h>
#include <WebCore/EventNames.h>
#include <WebCore/TextCheckerClient.h>
#include <WebCore/Document.h>
#include <WebCore/Editor.h>
#include <WebCore/PlatformKeyboardEvent.h>
#include <WebCore/Frame.h>


//////////////////////////////////////////////////////////////////////////




namespace wke
{




class EditorClient : public WebCore::EditorClient, public WebCore::TextCheckerClient
{
public:
    virtual ~EditorClient();

    virtual void pageDestroyed() override;

    virtual bool shouldDeleteRange(WebCore::Range*) override;

    virtual bool shouldShowDeleteInterface(WebCore::HTMLElement*) override;

    virtual bool smartInsertDeleteEnabled() override;

    virtual bool isSelectTrailingWhitespaceEnabled() override;

    virtual bool isContinuousSpellCheckingEnabled() override;

    virtual void toggleContinuousSpellChecking() override;

    virtual bool isGrammarCheckingEnabled() override;

    virtual void toggleGrammarChecking() override;

    virtual int spellCheckerDocumentTag() override;

    virtual bool shouldBeginEditing(WebCore::Range*) override;

    virtual bool shouldEndEditing(WebCore::Range*) override;

    virtual bool shouldInsertNode(WebCore::Node*, WebCore::Range*, WebCore::EditorInsertAction) override;

    virtual bool shouldInsertText(const String&, WebCore::Range*, WebCore::EditorInsertAction) override;

    virtual bool shouldChangeSelectedRange(WebCore::Range* fromRange, WebCore::Range* toRange, WebCore::EAffinity, bool stillSelecting) override;

    virtual bool shouldApplyStyle(WebCore::CSSStyleDeclaration*, WebCore::Range*) override;

    virtual bool shouldMoveRangeAfterDelete(WebCore::Range*, WebCore::Range*) override;

    virtual void didBeginEditing() override;

    virtual void respondToChangedContents() override;

    virtual void respondToChangedSelection() override;

    virtual void didEndEditing() override;

    virtual void didWriteSelectionToPasteboard() override;

    virtual void didSetSelectionTypesForPasteboard() override;

    virtual void registerCommandForUndo(PassRefPtr<WebCore::EditCommand>) override;

    virtual void registerCommandForRedo(PassRefPtr<WebCore::EditCommand>) override;

    virtual void clearUndoRedoOperations() override;

    virtual bool canCopyCut(WebCore::Frame*, bool defaultValue) const override;

    virtual bool canPaste(WebCore::Frame*, bool defaultValue) const override;

    virtual bool canUndo() const override;

    virtual bool canRedo() const override;

    virtual void undo() override;

    virtual void redo() override;

    virtual void handleKeyboardEvent(WebCore::KeyboardEvent* evt) override;

    virtual void handleInputMethodKeydown(WebCore::KeyboardEvent*) override;

    virtual void textFieldDidBeginEditing(WebCore::Element*) override;

    virtual void textFieldDidEndEditing(WebCore::Element*) override;

    virtual void textDidChangeInTextField(WebCore::Element*) override;

    virtual bool doTextFieldCommandFromEvent(WebCore::Element*, WebCore::KeyboardEvent*) override;

    virtual void textWillBeDeletedInTextField(WebCore::Element*) override;

    virtual void textDidChangeInTextArea(WebCore::Element*) override;

    virtual void ignoreWordInSpellDocument(const String&) override;

    virtual void learnWord(const String&) override;

    virtual void checkSpellingOfString(const UChar*, int length, int* misspellingLocation, int* misspellingLength) override;

    virtual String getAutoCorrectSuggestionForMisspelledWord(const String& misspelledWord) override;

    virtual void checkGrammarOfString(const UChar*, int length, Vector<WebCore::GrammarDetail>&, int* badGrammarLocation, int* badGrammarLength) override;

    virtual void updateSpellingUIWithGrammarString(const String&, const WebCore::GrammarDetail& detail) override;

    virtual void updateSpellingUIWithMisspelledWord(const String&) override;

    virtual void showSpellingUI(bool show) override;

    virtual bool spellingUIIsShowing() override;

    virtual void willSetInputMethodState() override;

    virtual void setInputMethodState(bool enabled) override;

    virtual void getGuessesForWord(const WTF::String& word, const WTF::String& context, WTF::Vector<WTF::String>& guesses) override;

    virtual void requestCheckingOfString(WebCore::SpellChecker*, int, WebCore::TextCheckingTypeMask, const WTF::String&) override;

    virtual WebCore::TextCheckerClient* textChecker() override;
};




};//namespace wke


#endif//#ifndef WKE_EDITOR_CLIENT_H