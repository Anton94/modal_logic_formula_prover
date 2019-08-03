describe('Protractor Demo App', function() {
  it('should have a title', function() {
    browser.get('http://localhost:34567/index.html');
    expect(browser.getTitle()).toEqual('Super Calculator');
  });
});