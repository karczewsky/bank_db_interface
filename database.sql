create table if not exists client
(
	client_id serial not null
		constraint client_pkey
			primary key,
	first_name varchar(30) not null,
	last_name varchar(50) not null,
	gender integer not null,
	city varchar(30) not null,
	street varchar(50) not null,
	postal_code char(6) not null,
	email varchar(40)
);

alter table client owner to postgres;

create table if not exists account
(
	account_id serial not null
		constraint account_pkey
			primary key,
	client_id integer not null
		constraint account_client_client_id_fk
			references client
				on delete cascade,
	money integer default 0 not null,
	max_debit integer default 0 not null
);

alter table account owner to postgres;

create table if not exists transaction
(
	transaction_id serial not null
		constraint transaction_pkey
			primary key,
	account_id integer not null
		constraint transaction_account_account_id_fk
			references account
				on delete cascade,
	date timestamp default now() not null,
	description varchar(50),
	amount integer not null
);

alter table transaction owner to postgres;

