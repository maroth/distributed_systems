parclass HelloWorld
{
  classuid(1000);

  public:
    HelloWorld();

    sync conc void setID(int i);
    sync conc int getID();

  private:
    int id;
};
