parclass HelloWorld
{
  classuid(1000);

  public:
    HelloWorld();

    sync seq void setID(int i);
    sync seq int getID();

  private:
    int id;
};

/*
    async seq void setID(int i);
    sync seq int getID();

    async seq void setID(int i);
    sync conc int getID();

    async conc void setID(int i);
    sync seq int getID();

    async conc void setID(int i);
    sync conc int getID();

    sync seq void setID(int i);
    sync seq int getID();

    sync seq void setID(int i);
    sync conc int getID();

    sync conc void setID(int i);
    sync seq int getID();

    sync conc void setID(int i);
    sync conc int getID();
*/
