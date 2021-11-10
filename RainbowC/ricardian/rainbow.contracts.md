<h1 class="contract">approve</h1>

---
spec_version: "0.2.0"
title: Approve Created Token
summary: 'Approve characteristics of a newly created token, or reject it'
icon: @ICON_BASE_URL@/@TOKEN_ICON_URI@
---

The contract owner allows the issuer to begin issuing tokens under a newly created {{symbol_to_symbol_code symbol}}.
Once approved, the issuer may modify the token configuration without any further approval action required.
If {{reject_and_clear}} is true, and there are no outstanding issued tokens, the token is deleted.

<<h1 class="contract">close</h1>

---
spec_version: "0.2.0"
title: Close Token Balance
summary: 'Close {{nowrap owner}}’s zero quantity balance'
icon: @ICON_BASE_URL@/@TOKEN_ICON_URI@
---

{{owner}} agrees to close their zero quantity balance for the {{symbol_to_symbol_code symbol}} token.

RAM will be refunded to the RAM payer of the {{symbol_to_symbol_code symbol}} token balance for {{owner}}.

<h1 class="contract">create</h1>

---
spec_version: "0.2.0"
title: Create or Reconfigure Token
summary: 'Create a new token, or reconfigure an existing token'
icon: @ICON_BASE_URL@/@TOKEN_ICON_URI@
---

{{issuer}} agrees to create a new token with the following characteristics to be managed by {{issuer}}:
symbol {{asset_to_symbol_code maximum_supply}}, {{membership_mgr}},
{{withdrawal_mgr}}, {{withdraw_to}}, {{freeze_mgr}}, {{redeem_locked_until}}, {{config_locked_until}}.

If this action is executed on an existing token, is authorized by the issuer, and the existing config_locked_until 
value is in the past, the token characteristics will be updated.

This action will not result any any tokens being issued into circulation.

{{issuer}} will be allowed to issue tokens into circulation, up to a maximum supply of {{maximum_supply}}.

RAM will deducted from {{issuer}}’s resources to create the necessary records.

<h1 class="contract">freeze</h1>

---
spec_version: "0.2.0"
title: Freeze Token Transfers
summary: 'Prevent non-Adminsitrative Token Transfers'
icon: @ICON_BASE_URL@/@TOKEN_ICON_URI@
---

If {{freeze}} is true, transfers are suspended; if false, transfers are enabled.
The freeze_mgr account configured during the `create` action must authorize this action.

Note that token issuance and withdrawals by the withdrawal_mgr cannot be frozen.

<h1 class="contract">issue</h1>

---
spec_version: "0.2.0"
title: Issue Tokens into Circulation
summary: 'Issue {{nowrap quantity}} into circulation and transfer into issuer’s account'
icon: @ICON_BASE_URL@/@TOKEN_ICON_URI@
---

The token manager agrees to issue {{quantity}} into circulation, and transfer it into the issuer account specified in the stats table. Issuance is not permitted until the approve action has been executed.

{{#if memo}}There is a memo attached to the transfer stating:
{{memo}}
{{/if}}

RAM will be deducted from the token manager (issuer) resources to create the necessary records.

This action does not allow the total quantity to exceed the max allowed supply of the token.

A proportionate number of staking tokens are transferred from issuer's account to the stake_to escrow account for each non-deferred stake listed in the stake stats table.

<h1 class="contract">open</h1>

---
spec_version: "0.2.0"
title: Open Token Balance
summary: 'Open a zero quantity balance for {{nowrap owner}}'
icon: @ICON_BASE_URL@/@TOKEN_ICON_URI@
---

{{ram_payer}} agrees to establish a zero quantity balance for {{owner}} for the {{symbol_to_symbol_code symbol}} token. As a result, RAM will be deducted from {{ram_payer}}’s resources to create the necessary records.

The membership_mgr account configured during the create action must authorize this action, unless the membership_mgr account has been configured as "allowallacct".

<h1 class="contract">resetram</h1>

---
spec_version: "0.2.0"
title: Reset Table RAM
summary: 'Clears rows of a RAM table named {{name}} with {{scope}}'
icon: @ICON_BASE_URL@/@TOKEN_ICON_URI@
---

Contract owner agrees to erase all rows of the specified table and scope. As a result, nonzero balances may be destroyed.

This action is intended for use during development or disaster recovery only.

<h1 class="contract">retire</h1>

---
spec_version: "0.2.0"
title: Remove Tokens from Circulation
summary: 'Remove {{nowrap quantity}} from circulation'
icon: @ICON_BASE_URL@/@TOKEN_ICON_URI@
---

The {{nowrap owner}} agrees to remove {{quantity}} from circulation, taken from their own account.
If configuration redeem_locked_until is in the past, any owner may execute; if not, only the issuer may retire tokens.

A proportionate number of staking tokens are transferred from the stake_to escrow account to {{owner}}'s account

{{#if memo}} There is a memo attached to the action stating:
{{memo}}
{{/if}}

<h1 class="contract">setstake</h1>

---
spec_version: "0.2.0"
title: Create or Update a Staking Relationship for a Token
summary: 'Set staking configuration'
icon: @ICON_BASE_URL@/@TOKEN_ICON_URI@
---

{{issuer}} agrees to associate a staking contract, with the following characteristics, to a token currently managed by {{issuer}}: {{asset_to_symbol_code token_bucket}}, {{asset_to_symbol_code stake_per_bucket}}, {{stake_token_contract}}, {{stake_to}}. The staking ratio (staked tokens per issued token) equals the stake_per_bucket quantity divided by the token_bucket quantity.

If this action is executed on an existing token, is authorized by the issuer, the config_locked status is false, and the updated staking configuration would not exceed the maximum allowed count of stake relationships,
the staking configuration will be updated.

RAM will deducted from {{issuer}}’s resources to create the necessary records.

If a new staking relationship is created, or if the {{stake_per_token}} amount of an existing relationship is increased from a value of zero ("restaked"), a proportionate number of staking tokens are transferred from the issuer's account to the {{stake_to}} escrow account.

If the {{stake_per_token}} amount of an existing relationship is decreased to a value of zero ("destaked"), a proportionate number of staking tokens are transferred from the stake_to escrow account to the issuer's account.

If the {{stake_to}} account is the special name "deletestake", the staking relationship is removed from the stake table.

{{#if memo}} There is a memo attached to the action stating:
{{memo}}
{{/if}}

<h1 class="contract">transfer</h1>

---
spec_version: "0.2.0"
title: Transfer Tokens
summary: 'Send {{nowrap quantity}} from {{nowrap from}} to {{nowrap to}}'
icon: @ICON_BASE_URL@/@TRANSFER_ICON_URI@
---

Caution: this contract text may not comply with the ricardian spec.

{{quantity}} is transferred from {{from}} to {{to}} under the conditions below.

Required Condition 1. either 
  (a) {{to}} has been enabled by the membership_mgr by the `open` action, or
  (b) the membership criterion has been disabled by setting membership_mgr to "allowallacct" in the `create`
  action.
Required Condition 2. either
  (a) {{from}} is the issuer, or
  (b) both [i] {{from}} has authorized the action AND [ii] transactions are NOT frozen, or
  (c) both [i] the withdrawal_mgr has authorized the action AND [ii] {{to}} is the withdraw_to account.

{{#if memo}}There is a memo attached to the transfer stating:
{{memo}}
{{/if}}

If {{from}} is not already the RAM payer of their {{asset_to_symbol_code quantity}} token balance, {{from}} will be designated as such. As a result, RAM will be deducted from {{from}}’s resources to refund the original RAM payer.

If {{to}} does not have a balance for {{asset_to_symbol_code quantity}}, {{from}} will be designated as the RAM payer of the {{asset_to_symbol_code quantity}} token balance for {{to}}. As a result, RAM will be deducted from {{from}}’s resources to create the necessary records.
