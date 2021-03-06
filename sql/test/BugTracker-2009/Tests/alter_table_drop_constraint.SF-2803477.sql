CREATE TABLE TABLE_A (
    TABLE_A_ID INT NOT NULL,
    VALUE_A INT NOT NULL,
    CONSTRAINT PK_TABLE_A_ID PRIMARY KEY ( TABLE_A_ID )
);

CREATE TABLE TABLE_B (
    TABLE_B_ID INT NOT NULL,
    TABLE_A_ID INT NOT NULL,
    CONSTRAINT PK_TABLE_B_ID PRIMARY KEY ( TABLE_B_ID )
);

CREATE TABLE TABLE_D (
    TABLE_D_ID INT NOT NULL,
    TABLE_A_ID INT NOT NULL,
    CONSTRAINT PK_TABLE_D_ID PRIMARY KEY ( TABLE_D_ID ),
    CONSTRAINT FK_TABLE_D_TABLE_A_ID FOREIGN KEY ( TABLE_A_ID ) REFERENCES TABLE_A ( TABLE_A_ID )
);

CREATE TABLE TABLE_C (
    TABLE_C_ID INT NOT NULL,
    TABLE_A_ID INT NOT NULL,
    TABLE_B_ID INT NOT NULL,
    TABLE_D_ID INT NOT NULL,
    VALUE_C    INT NOT NULL,
    CONSTRAINT PK_TABLE_C_ID PRIMARY KEY ( TABLE_C_ID ),
    CONSTRAINT UNQ_VALUE_C UNIQUE ( VALUE_C ),
    CONSTRAINT FK_TABLE_C_TABLE_A_ID FOREIGN KEY ( TABLE_A_ID ) REFERENCES TABLE_A ( TABLE_A_ID ),
    CONSTRAINT FK_TABLE_C_TABLE_D_ID FOREIGN KEY ( TABLE_D_ID ) REFERENCES TABLE_B ( TABLE_B_ID )
);

ALTER TABLE TABLE_B
    DROP CONSTRAINT UNQ_VALUE_C;

ALTER TABLE TABLE_B
    ADD CONSTRAINT FK_TABLE_A_ID FOREIGN KEY ( TABLE_A_ID ) REFERENCES TABLE_A ( TABLE_A_ID );
