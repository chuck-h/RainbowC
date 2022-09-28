# Rainbow Contract

*This contract is now included as `seeds.rainbows.cpp` in https://github.com/chuck-h/seeds-smart-contracts . Ongoing development occurs there.*

The Rainbow Contract is a tool which supports digital *tokens* on the Telos blockchain. It is a flexible tool, and offers
several different configuration options targeted towards different types of use cases. This Rainbow Contract was
inspired by a concept for locally-controlled currency within the SEEDS ecosystem.

Because of its flexibility, this Rainbow Contract may be useful as a "prototyping tool" for projects which anticipate
evolving towards optimum token behavior through a series of trials.

For more information on the Telos blockchain, see https://www.telos.net/ and https://eos.io/ . For more information on the SEEDS
project, see https://joinseeds.earth and https://medium.com/joinseeds/rainbow-seeds-v1-0-combining-the-best-of-local-and-global-currencies-for-a-regenerative-economy-1fb713efd9b1 .

## Background on tokens
Users, who may be individuals or organizations, have *accounts* on the blockhain. Each account can possess many *tokens*
of different types and in different quantities. A token typically has meaning in the context of a game, a loyalty program, or
an economic project. A token may represent a fanciful object, e.g. a magical sword within a game; it may represent a unit of
loyalty credit, e.g. coffees purchased at a particular stand, or miles travelled on an airline; it may represent a unit of
exchangeable value, e.g. within a festival event or as a town's local currency. Some tokens (*not* those we create with the
Rainbow Contract) already exist on the network, are widely traded around the world, and represent millions of dollars in value.

Without a blockchain, organizations have managed programs like these using physical tickets and with data maintained in a central
ledger. Blockchain technology provides a method of token bookkeeping which is *transparent* (anyone can see the "books") and
*permanent* (nobody can alter the "books"). Blockchain provides a reliable means for individuals to *transfer* digital tokens that
is functionally equivalent to passing a physical ticket or banknote from one person to another. Thus many types of gaming and
economic activities can be represented with these tokens.

Two examples of blockchain tokens in the public eye are *Bitcoin*, which is the first digital blockchain currency, and *NFTs* (Non
Fungible Tokens) which have been used to represent unique digital artworks.

## The role of the Rainbow Contract
The Rainbow Contract is a tool which enables a "master" user (the *issuer*) to create tokens and manage token bookkeeping (i.e.
keep track of which users own how many units). It is used in connection with *applications* which provide meaningful user
experiences based on the tokens. These applications rely on *blockchain libraries* such as eosjs (https://developers.eos.io/manuals/eosjs/latest/index)
to interact with the Rainbow Contract.

A host organization may wish to create several distinct tokens within its purview. This host organization would install one
copy of this Rainbow Contract on a dedicated blockchain account (the *contract account*) that the organization owns. Installing
the contract does not yet create any tokens.

Another blockchain account owned by the organization then becomes the *issuer* of a new token, by executing the `create` action of the Rainbow Contract.
When creating this new token, the issuer chooses several *configuration characteristics*, for example whether the user can trade
it (like a coin), or simply hold it (like a diploma). An important characteristic is whether or not the token is *backed*. When each
"backed" token is first made, it is associated with an amount of value in units of a second token. This second token, for example, might be a
widely recognized currency token on the blockchain, e.g. a stablecoin like USDC -- representing US dollars -- or an ecosystem
currency like Seeds. By means of this *backing*, the issuer's new token has a "face value" or "embedded value" in the backing
currency. The issuer may establish rules under which their token may be "cashed in" for its face value.

The issuer has godlike powers over the tokens they create, matching or even exceeding the powers of a central bank over a
national currency. When an organization creates a family of tokens, it is convenient to use the same issuing account to
manage all of them. However it is equally possible to maintain different blockchain accounts (with different individuals
controlling them) to supervise different tokens.

Tokens created under the Rainbow Contract conform with the expected behavior of other tokens on the Telos blockchain. They can
be held and transferred between users by publicly available "wallets" such as Anchor, Sqrl, etc. However in most applications,
the organization hosting the tokens will develop its own family of applications through which their end users interact. These
users may not even realize that the "points", "quadroons", or "merit badges" they see are represented as tokens on a blockchain.
