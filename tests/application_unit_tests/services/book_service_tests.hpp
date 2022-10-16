#pragma once
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QString>
#include <QSignalSpy>
#include <QUuid>
#include "book.hpp"
#include "book_meta_data.hpp"
#include "book_operation_status.hpp"
#include "i_book_info_helper.hpp"
#include "book_service.hpp"
#include "tag.hpp"


using namespace testing;
using ::testing::ReturnRef;
using application::BookOperationStatus;
using namespace application::services;
using namespace application;
using namespace domain::models;


namespace tests::application
{

class BookInfoHelperMock : public IBookInfoHelper
{
public:
    MOCK_METHOD(std::optional<BookMetaData>, getBookMetaData, 
                (const QString&), (override));
    MOCK_METHOD(void, getCover, (), (const, override));
};


struct ABookService : public ::testing::Test
{
    void SetUp() override
    {
        EXPECT_CALL(bookInfoHelperMock, getBookMetaData(_))
                .WillRepeatedly(Return(BookMetaData()));
        
        bookService = std::make_unique<BookService>(&bookInfoHelperMock);
    }
    
    BookInfoHelperMock bookInfoHelperMock;
    std::unique_ptr<BookService> bookService;
};



TEST_F(ABookService, SucceedsAddingABook)
{
    // Arrange
    auto expectedResult = BookOperationStatus::Success;
    
    
    // Act
    auto result = bookService->addBook("some/path.pdf");
    
    // Assert
    EXPECT_EQ(expectedResult, result);
}



TEST_F(ABookService, SucceedsDeletingABook)
{
    // Arrange
    bookService->addBook("some/path.pdf");
    bookService->addBook("some/other/path.pdf");
    const auto& firstBookUuid = bookService->getBooks()[0].getUuid();
    
    auto expectedResult = BookOperationStatus::Success;
    
    
    // Act
    auto preDeleteBookCount = bookService->getBookCount();
    auto result = bookService->deleteBook(firstBookUuid);
    
    // Assert
    EXPECT_EQ(expectedResult, result);
    EXPECT_EQ(preDeleteBookCount - 1, bookService->getBookCount());
}

TEST_F(ABookService, FailsDeletingABookIfBookDoesNotExist)
{
    // Arrange
    auto nonExistentBookUuid = QUuid::createUuid();
    
    auto expectedResult = BookOperationStatus::BookDoesNotExist;
    
    
    // Act
    bookService->addBook("some/first.pdf");
    
    auto result = bookService->deleteBook(nonExistentBookUuid);
    
    // Assert
    EXPECT_EQ(expectedResult, result);
}



TEST_F(ABookService, SucceedsUpdatingABook)
{
    // Arrange
    QSignalSpy spy(bookService.get(), &BookService::dataChanged);
    
    QString originalPath = "/some/path.pdf";
    BookMetaData bookMetaData { .title = "SomeBook", .author = "SomeAuthor" };
    
    BookMetaData newBookMetaData{ .title = "ANewTitle", .author = "ANewAuthor" };
    Book bookToUpdateWith("some/path", newBookMetaData);
    bookToUpdateWith.addTag(Tag("FirstTag"));
    bookToUpdateWith.addTag(Tag("FirstTag"));
    
    auto expectedStatus = BookOperationStatus::Success;
    auto expectedResult = bookToUpdateWith;
    
    
    // Expect
    EXPECT_CALL(bookInfoHelperMock, getBookMetaData(_))
            .Times(1)
            .WillOnce(Return(bookMetaData));
    
    // Act
    bookService->addBook(originalPath);
    const QUuid& bookUuid = bookService->getBooks()[0].getUuid();
    
    auto resultStatus = bookService->updateBook(bookUuid, bookToUpdateWith);
    auto result = bookService->getBook(bookUuid);
    
    // Assert
    EXPECT_EQ(expectedStatus, resultStatus);
    EXPECT_EQ(expectedResult.getTitle(), result->getTitle());
    EXPECT_EQ(expectedResult.getFilePath(), result->getFilePath());
    
    for(int i = 0; i < expectedResult.getTags().size(); ++i)
    {
        EXPECT_EQ(expectedResult.getTags()[i], result->getTags()[i]);
    }
    
    auto arguments = spy[0];
    EXPECT_EQ(1, spy.count());
    EXPECT_EQ(bookService->getBookIndex(bookUuid), arguments[0].toInt());
}

TEST_F(ABookService, FailsUpdatingABookIfBookDoesNotExist)
{
    // Arrange
    QString bookUuid = "non-existend-uuid";
    Book bookToUpdateWidth("some/path", BookMetaData());
    
    auto expectedStatus = BookOperationStatus::BookDoesNotExist;
    
    
    // Act
    auto resultStatus = bookService->updateBook(bookUuid, bookToUpdateWidth);
    
    // Assert
    EXPECT_EQ(expectedStatus, resultStatus);
}



TEST_F(ABookService, SucceedsGettingABook)
{
    // Arrange
    QString path = "some/path.pdf";
    BookMetaData bookMetaData{ .title = "SomeTitle", .author = "SomeAuthor" };
    Book expectedResult(path, bookMetaData);
    
    
    // Expect
    EXPECT_CALL(bookInfoHelperMock, getBookMetaData(_))
            .Times(1)
            .WillOnce(Return(bookMetaData));
    
    // Act
    bookService->addBook(path);
    const auto& bookUuid = bookService->getBooks()[0].getUuid();
    
    auto result = bookService->getBook(bookUuid);
    
    // Assert
    EXPECT_EQ(expectedResult.getTitle(), result->getTitle());
    EXPECT_EQ(expectedResult.getAuthor(), result->getAuthor());
    EXPECT_EQ(expectedResult.getFilePath(), result->getFilePath());
}

TEST_F(ABookService, FailsGettingABookIfBookDoesNotExist)
{
    // Arrange
    QString bookUuid = "non-existend-uuid";
    
    auto expectedResult = nullptr;
    
    
    // Act
    auto result = bookService->getBook(bookUuid);
    
    // Assert
    EXPECT_EQ(expectedResult, result);
}



TEST_F(ABookService, SucceedsAddingATag)
{
    // Arrange
    Tag firstTag("FirstTag");
    Tag secondTag("SecondTag");
    auto expectedResultStatus = BookOperationStatus::Success;
    
    
    // Act
    bookService->addBook("some/path.pdf");
    const auto& bookUuid = bookService->getBooks()[0].getUuid();
    
    auto firstResultStatus = bookService->addTag(bookUuid, firstTag);
    auto secondResultStatus = bookService->addTag(bookUuid, secondTag);
    
    auto result = bookService->getBook(bookUuid);
    
    // Assert
    EXPECT_EQ(expectedResultStatus, firstResultStatus);
    EXPECT_EQ(expectedResultStatus, secondResultStatus);
    
    EXPECT_EQ(firstTag, result->getTags()[0]);
    EXPECT_EQ(secondTag, result->getTags()[1]);
}

TEST_F(ABookService, FailsAddingATagIfTagAlreadyExists)
{
    // Arrange
    Tag tag("FirstTag");
    auto expectedResult = BookOperationStatus::TagAlreadyExists;
    
    
    // Act
    bookService->addBook("some/path.pdf");
    const auto& bookUuid = bookService->getBooks()[0].getUuid();
    
    bookService->addTag(bookUuid, tag);
    auto result = bookService->addTag(bookUuid, tag);
    
    
    // Assert
    EXPECT_EQ(expectedResult, result);
}

TEST_F(ABookService, FailsAddingATagIfBookDoesNotExist)
{
    // Arrange
    QUuid bookUuid = "non-existend-uuid";
    Tag firstTag("FirstTag");
    
    auto expectedResult = BookOperationStatus::BookDoesNotExist;
    
    
    auto result = bookService->addTag(bookUuid, firstTag);
    
    // Assert
    EXPECT_EQ(expectedResult, result);
}


TEST_F(ABookService, SucceedsGettingAllBooks)
{
    // Arrange
    BookMetaData firstBookMetaData{ .title = "FirstBook", .author = "Author1" };
    Book firstBook("/some/path", firstBookMetaData);
    BookMetaData secondBookMetaData{ .title = "SecondBook", .author = "Author2" };
    Book secondBook("/some/path2", secondBookMetaData);
    BookMetaData thirdBookMetaData{ .title = "ThirdBook", .author = "Author3" };
    Book thirdBook("/some/path2", thirdBookMetaData);
    
    std::vector<Book> expectedResult { firstBook, secondBook, thirdBook };
    
    
    // Expect
    EXPECT_CALL(bookInfoHelperMock, getBookMetaData(_))
            .Times(3)
            .WillOnce(Return(firstBookMetaData))
            .WillOnce(Return(secondBookMetaData))
            .WillOnce(Return(thirdBookMetaData));
    
    // Act
    bookService->addBook(firstBook.getFilePath());
    bookService->addBook(secondBook.getFilePath());
    bookService->addBook(thirdBook.getFilePath());
    
    auto results = bookService->getBooks();
    
    // Assert
    for(int i = 0; i < expectedResult.size(); ++i)
    {
        EXPECT_EQ(expectedResult[i].getTitle(), results[i].getTitle());
        EXPECT_EQ(expectedResult[i].getAuthor(), results[i].getAuthor());
        EXPECT_EQ(expectedResult[i].getFilePath(), results[i].getFilePath());
    }
}

TEST_F(ABookService, SucceedsGettingTheBookCount)
{
    // Arrange
    auto expectedResult = 2;
    
    
    // Act
    bookService->addBook("some/path.pdf");
    bookService->addBook("some/other/path.pdf");
    
    auto result = bookService->getBookCount();
    
    // Assert
    EXPECT_EQ(expectedResult, result);
}



TEST_F(ABookService, SucceedsRefreshingLastOpenedFlag)
{
    // Arrange
    bookService->addBook("some/path.pdf");
    const auto& bookUuid = bookService->getBooks()[0].getUuid();
    
    // Act
    auto before = bookService->getBook(bookUuid)->getLastOpened();
    bookService->refreshLastOpenedFlag(bookUuid);
    auto after = bookService->getBook(bookUuid)->getLastOpened();
    
    // Assert
    EXPECT_NE(before, after);
}

} // namespace tests::application