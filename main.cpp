#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <sstream>
#include <random>

using namespace std;



class Contact {
private:
	static int nextID;
	int id;
public:
	// Name of Contact
	string name;
	// Hashmap, string key (type), and holds vector of strings (numbers)
	unordered_map<string, vector<string>> phone_numbers;

	Contact() {
		id = nextID++;
	}

	Contact(const string& name) : name(name), id(nextID++) {}

	int getID() const { return id; }

	void addPhoneNumber(const string& type, const string& number) {
		// Check if the phone number already exists for the given type
		auto it = phone_numbers.find(type);
		if (it != phone_numbers.end()) {
			// Check if the number already exists
			if (find(it->second.begin(), it->second.end(), number) != it->second.end()) {
				cout << "Phone number already exists for type " << type << ": " << number << endl;
				return;
			}
		}
		// If the phone number doesn't exist, add it
		phone_numbers[type].push_back(number);
	}

	void display() const {
		cout << "ID: " << id << endl;
		cout << "Name: " << name << endl;
		for (const auto& entry : phone_numbers) {
			cout << entry.first << " Numbers:" << endl;
			for (const auto& number : entry.second) {
				cout << "- " << number << endl;
			}
		}
		cout << endl;
	}

	string serialize() const {
		string serialized = name + "\n";
		for (const auto& entry : phone_numbers) {
			for (const auto& number : entry.second) {
				serialized += entry.first + "," + number + "\n";
			}
		}
		serialized += "\n";
		return serialized;
	}
};

int Contact::nextID = 0;

class ContactDirectory {
private:
	unordered_map<string, Contact> contacts;
	unordered_map<string, int> contactIDs;
	// Map to store contacts using a unique string key

	// Generate a unique key for a contact name (handles duplicates)
	string generateUniqueKey(const string& name) {
		string key = name;
		transform(key.begin(), key.end(), key.begin(), ::tolower); // Convert name to lowercase for case-insensitive matching
		int id = 1;
		while (contacts.find(key) != contacts.end()) {
			key = name + "_" + to_string(id++);
		}
		return key;
	}


	Contact deserializeContact(const string& data) const {
		stringstream ss(data);
		string name;
		getline(ss, name);

		Contact contact(name);
		string line;
		while (getline(ss, line)) {
			stringstream line_ss(line);
			string type, number;
			getline(line_ss, type, ',');
			getline(line_ss, number);
			contact.addPhoneNumber(type, number);
		}
		return contact;
	}

	string serializeContacts() const {
		stringstream ss;
		for (const auto& entry : contacts) {
			ss << entry.second.serialize();
		}
		return ss.str();
	}

public:

	int size() const {  // Added size method
		return contacts.size();
	}

	// Add contact using a generated unique key
	void addContact(const Contact& contact) {
		for (const auto& entry : contacts) {
			if (entry.second.name == contact.name) {
				cout << "Duplicate contact name found. Choose an option:" << endl;
				cout << "1. Merge with existing contact" << endl;
				cout << "2. Add as a new contact" << endl;
				cout << "Enter your choice: ";
				int choice;
				cin >> choice;
				cin.ignore(); // Ignore the newline character left in the input buffer
				if (choice == 1) {
					// Show a selection screen with the IDs of existing contacts with the same name
					cout << "Select the contact ID to merge with:" << endl;
					int index = 1;
					for (const auto& entry : contacts) {
						if (entry.second.name == contact.name) {
							cout << index << ". ID: " << entry.second.getID() << ", Name: " << entry.second.name << endl;
							index++;
						}
					}
					int selectedID;
					cin >> selectedID;
					cin.ignore(); // Ignore the newline character left in the input buffer

					// Merge contact information with the selected existing contact
					index = 1;
					for (auto& entry : contacts) {
						if (entry.second.name == contact.name) {
							if (selectedID == entry.second.getID()) {
								contacts[entry.first].phone_numbers.insert(contact.phone_numbers.begin(), contact.phone_numbers.end());
								cout << "Contact merged successfully." << endl;
								return;
							}
						}
					}
					cout << "Invalid contact ID. Contact not merged." << endl;
				}
				else if (choice == 2) {
					// Generate a new key for the new contact
					string key = generateUniqueKey(contact.name);
					contacts[key] = contact;
					cout << "New contact added successfully." << endl;
				}
				else {
					cout << "Invalid choice. Contact not added." << endl;
				}
				return; // Exit the method after handling the duplicate
			}
		}

		// If no duplicate name is found, add the contact as usual
		string key = generateUniqueKey(contact.name);
		contacts[key] = contact;
		cout << "Contact added successfully." << endl;
	}

	void deleteContact(ContactDirectory& directory, const string& name) {
		// Search for contacts with the given name
		vector<string> matchingKeys;
		for (const auto& entry : directory.contacts) {
			if (entry.second.name == name) {
				matchingKeys.push_back(entry.first);
			}
		}

		if (matchingKeys.empty()) {
			cout << "Contact with name '" << name << "' not found!" << endl;
			return;
		}

		if (matchingKeys.size() == 1) {
			// If only one contact with the given name exists, delete it directly
			directory.contacts.erase(matchingKeys.front());
			cout << "Contact deleted successfully." << endl;
		}
		else {
			// If multiple contacts with the given name exist, prompt the user to choose one
			cout << "Multiple contacts found with name '" << name << "'. Please choose an ID:" << endl;
			for (const auto& key : matchingKeys) {
				cout << "ID: " << directory.contacts[key].getID() << ", Name: " << directory.contacts[key].name << endl;
			}

			int choiceID;
			cout << "Enter the ID of the contact you want to delete: ";
			cin >> choiceID;

			// Find the chosen contact by ID
			auto it = find_if(matchingKeys.begin(), matchingKeys.end(),
				[&directory, choiceID](const string& key) {
					return directory.contacts[key].getID() == choiceID;
				});

			if (it != matchingKeys.end()) {
				// Delete the chosen contact
				directory.contacts.erase(*it);
				cout << "Contact deleted successfully." << endl;
			}
			else {
				cout << "Invalid ID. No contact deleted." << endl;
			}
		}
	}

	void searchContact(const string& name, int page, int pageSize) const {
		vector<const Contact*> searchResults;
		string lowercaseName = name;
		transform(lowercaseName.begin(), lowercaseName.end(), lowercaseName.begin(), ::tolower);
		for (const auto& entry : contacts) {
			string lowercaseEntryName = entry.second.name;
			transform(lowercaseEntryName.begin(), lowercaseEntryName.end(), lowercaseEntryName.begin(), ::tolower);
			if (lowercaseEntryName.find(lowercaseName) != string::npos) {
				searchResults.push_back(&(entry.second));
			}
		}

		int total_pages = (searchResults.size() + pageSize - 1) / pageSize;
		if (page > total_pages) {
			cout << "Page " << page << " is out of range." << endl;
			return;
		}

		int start = (page - 1) * pageSize;
		int end = min(start + pageSize, static_cast<int>(searchResults.size()));

		if (start >= end) {
			cout << "No matching contacts found." << endl;
			return;
		}

		for (int i = start; i < end; ++i) {
			searchResults[i]->display();
		}

		cout << "Page " << page << "/" << total_pages << endl;
		if (page < total_pages) {
			cout << "Type 'n' for next page, 'p' for previous page or 'q' to quit." << endl;
			char command;
			cin >> command;
			if (command == 'n') {
				searchContact(name, page + 1, pageSize);
			}
		}
	}

	void deletePhoneNumber(ContactDirectory& directory, int contactID, const string& phoneNumber) {
		// Find the contact with the given ID
		auto it = find_if(directory.contacts.begin(), directory.contacts.end(),
			[contactID](const auto& entry) { return entry.second.getID() == contactID; });

		if (it != directory.contacts.end()) {
			// Contact found
			Contact& contact = it->second;

			// Check if the contact has the specified phone number
			bool found = false;
			for (auto& entry : contact.phone_numbers) {
				auto& numbers = entry.second;
				auto numIt = find(numbers.begin(), numbers.end(), phoneNumber);
				if (numIt != numbers.end()) {
					// Phone number found, erase it
					numbers.erase(numIt);
					found = true;
					break;
				}
			}

			if (found) {
				cout << "Phone number '" << phoneNumber << "' deleted from contact '" << contact.name << "'." << endl;
			}
			else {
				cout << "Phone number '" << phoneNumber << "' not found in contact '" << contact.name << "'." << endl;
			}
		}
		else {
			// Contact with the given ID not found
			cout << "Contact with ID '" << contactID << "' not found." << endl;
		}
	}

	void searchContactByNumber(const string& query, int page, int pageSize) const {
		vector<const Contact*> searchResults;
		for (const auto& entry : contacts) {
			if (entry.first.find(query) != string::npos) {
				searchResults.push_back(&(entry.second));
			}
			else {
				for (const auto& phoneNumbers : entry.second.phone_numbers) {
					for (const auto& number : phoneNumbers.second) {
						if (number.find(query) == 0) {
							searchResults.push_back(&(entry.second));
							break;
						}
					}
				}
			}
		}

		int total_pages = (searchResults.size() + pageSize - 1) / pageSize;
		if (page > total_pages) {
			cout << "Page " << page << " is out of range." << endl;
			return;
		}

		int start = (page - 1) * pageSize;
		int end = min(start + pageSize, static_cast<int>(searchResults.size()));

		if (start >= end) {
			cout << "No matching contacts found." << endl;
			return;
		}

		for (int i = start; i < end; ++i) {
			searchResults[i]->display();
		}

		cout << "Page " << page << "/" << total_pages << endl;
		if (page < total_pages) {
			cout << "Type 'n' for next page, 'p' for previous page or 'q' to quit." << endl;
			char command;
			cin >> command;
			if (command == 'n') {
				searchContactByNumber(query, page + 1, pageSize);
			}
		}
	}

	void renameContact(ContactDirectory& directory, const string& oldName, const string& newName) {
		// Search for contacts with the old name
		vector<Contact*> matchingContacts;
		for (auto& entry : directory.contacts) {
			if (entry.second.name == oldName) {
				matchingContacts.push_back(&(entry.second));
			}
		}

		if (matchingContacts.empty()) {
			cout << "Contact with name '" << oldName << "' not found!" << endl;
			return;
		}

		if (matchingContacts.size() == 1) {
			// If only one contact with the old name exists, simply rename it
			Contact* contact = matchingContacts.front();
			string oldKey = directory.generateUniqueKey(oldName);
			string newKey = directory.generateUniqueKey(newName);

			// Update the contact's name and move it to a new key
			contact->name = newName;
			directory.contacts[newKey] = move(*(contact));
			directory.contacts.erase(oldKey); // Remove the old key

			cout << "Contact renamed successfully." << endl;
		}
		else {
			// If multiple contacts with the old name exist, prompt the user to choose one
			cout << "Multiple contacts found with name '" << oldName << "'. Please choose an ID:" << endl;
			for (auto contact : matchingContacts) {
				cout << "ID: " << contact->getID() << ", Name: " << contact->name << endl;
			}

			int choiceID;
			cout << "Enter the ID of the contact you want to rename: ";
			cin >> choiceID;

			// Find the chosen contact by ID
			auto it = find_if(matchingContacts.begin(), matchingContacts.end(),
				[choiceID](const Contact* contact) { return contact->getID() == choiceID; });

			if (it != matchingContacts.end()) {
				// Rename the chosen contact
				Contact* contact = *it;
				string oldKey = directory.generateUniqueKey(oldName);
				string newKey = directory.generateUniqueKey(newName);

				// Update the contact's name and move it to a new key
				contact->name = newName;
				directory.contacts[newKey] = move(*(contact));
				directory.contacts.erase(oldKey); // Remove the old key

				cout << "Contact renamed successfully." << endl;
			}
			else {
				cout << "Invalid ID. No contact renamed." << endl;
			}
		}
	}

	void displayAllContacts(int page, int pageSize) const {
		int total_pages = (contacts.size() + pageSize - 1) / pageSize;
		if (page > total_pages) {
			cout << "Page " << page << " is out of range." << endl;
			return;
		}

		int start = (page - 1) * pageSize;
		int end = start + pageSize;
		int count = 0;

		for (const auto& entry : contacts) {
			if (count >= start && count < end) {
				entry.second.display();
			}
			count++;
			if (count >= end) break;
		}

		cout << "Page " << page << "/" << total_pages << endl;
		if (page < total_pages) {
			cout << "Type 'n' for next page, 'p' for previous page or 'q' to quit." << endl;
			char command;
			cin >> command;
			if (command == 'n') {
				displayAllContacts(page + 1, pageSize);
			}
		}
	}

	void saveToFile(const string& filename) const {
		// Clear existing content of the file
		ofstream clearFile(filename, ios::trunc);
		clearFile.close();

		//Save contents to file
		ifstream file(filename);
		if (file) {
			// File exists, check if the contents are the same
			string fileContents((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
			if (fileContents == serializeContacts()) {
				// File contents are the same, no need to save
				cout << "File contents unchanged. No need to save." << endl;
				return;
			}
			file.close();
		}

		// Save contacts to file
		ofstream outFile(filename);
		if (!outFile) {
			cerr << "Error opening file. " << endl;
			return;
		}

		for (const auto& entry : contacts) {
			outFile << entry.second.serialize();
		}
		outFile.close();

		cout << "Contacts saved to file." << endl;
	}

	void loadFromFile(const string& filename) {
		ifstream file(filename);
		if (!file) {
			cerr << "Error opening file. " << endl;
			return;
		}
		string line;
		string contactData;
		while (getline(file, line)) {
			if (line.empty()) {
				if (!contactData.empty()) {
					Contact contact = deserializeContact(contactData);
					addContact(contact);
					contactData.clear();
				}
			}
			else {
				contactData += line + "\n";
			}
		}
		file.close();
	}
};

void displayMenu() {
	cout << "Menu Options:" << endl;
	cout << "1. Add Contact" << endl;
	cout << "2. Delete Contact" << endl;
	cout << "3. Remove Phone Number" << endl;
	cout << "4. Search Contact" << endl;
	cout << "5. Rename Contact" << endl;
	cout << "6. Display All Contacts" << endl;
	cout << "7. Save Contacts to File" << endl;
	cout << "8. Load Contacts from File" << endl;
	cout << "9. Exit" << endl;
	cout << "10. Save and Exit" << endl;
	cout << "Enter your choice: ";
}

/*
Goals:
Deleting and removing contacts, phone numbers, etc...
	Chaning existing contact names (Not sure where to begin)

Sorting System, Alphabetical Last Names/First Names, by Phone Number. 
Searching System (If I were to search a phone number or name with a few characters it should show all possible results)
Menu System (To navigate within the terminal)
	Selecting phone numbers from the list and choosing what to do with them
Page List (Integrate a page system to navigate the large list of phone numbers)
Somehow implement duplicate contact name but unique identifier to distinguish difference
...
*/

int main() {
	ContactDirectory directory;
	// Load contacts from file
	
	
	// Display all contacts
	// directory.displayAllContacts();

	int choice;
	int pageSize = 5;
	int currentPage = 1;
	int id;
	string name, newName, type, number, filename;
	filename = "contacts.txt";
	directory.loadFromFile(filename);
	do {
		displayMenu();
		cin >> choice;
		cin.ignore();  // Ignore the newline character left in the input buffer

		switch (choice) {
		case 1:
			cout << "Enter contact name: ";
			getline(cin, name);
			cout << "Enter phone number type: ";
			getline(cin, type);
			cout << "Enter phone number: ";
			getline(cin, number);
			{
				Contact contact(name);
				contact.addPhoneNumber(type, number);
				directory.addContact(contact);
			}
			break;

		case 2:
			cout << "Enter contact name to delete: ";
			getline(cin, name);
			directory.deleteContact(directory, name);
			break;
		case 3:
			int sch;
			cout << "IDs are needed! \n" << "1. Continue \n" << "2+. Exit \n";
			cin >> sch;
			if (sch == 1) {
				cout << "Enter the ID: \n";
				cin >> id;
				cout << "Enter the Phone Number \n";
				cin >> number;
				directory.deletePhoneNumber(directory, id, number);
			}
			else {
				break;
			}

		case 4:
			int searchChoice;
			cout << "1. Search by Name" << endl << "2. Search by Number" << endl << "3+. Exit" << endl;
			cin >> searchChoice;
			cin.ignore();
			if (searchChoice == 1) {
				cout << "Enter contact name to search: ";
				getline(cin, name);
				do {
					directory.searchContact(name, currentPage, pageSize);
					cout << "Commands: [n] Next Page, [q] Quit" << endl;
					char command;
					cin >> command;
					if (command == 'n' && currentPage * pageSize < directory.size()) {
						currentPage++;
					}
					else if (command == 'q') {
						break; // Exit loop when 'q' is entered
					}
					else {
						cout << "Invalid command." << endl;
					}
				} while (true);
				break;
			}
			else if (searchChoice == 2) {
				cout << "Enter search query: ";
				getline(cin, number);
				do {
					directory.searchContactByNumber(number, currentPage, pageSize);
					cout << "Commands: [n] Next Page, [q] Quit" << endl;
					char command;
					cin >> command;
					if (command == 'n' && currentPage * pageSize < directory.size()) {
						currentPage++;
					}
					else if (command == 'q') {
						break; // Exit loop when 'q' is entered
					}
					else {
						cout << "Invalid command." << endl;
					}
				} while (true);
				break;
			}
			else {
				break;
			}

		case 5:
			cout << "Enter old contact name: ";
			getline(cin, name);
			cout << "Enter new contact name: ";
			getline(cin, newName);
			directory.renameContact(directory, name, newName);
			break;

		case 6:
			while(true) {
				directory.displayAllContacts(currentPage, pageSize);
				cout << "Commands: [n] Next Page, [p] Previous Page, [q] Quit" << endl;
				char command;
				cin >> command;
				if (command == 'n' && currentPage * pageSize < directory.size()) {
					currentPage++;
				}
				else if (command == 'p' && currentPage > 1) {
					currentPage--;
				}
				else if (command == 'q') {
					break;
				}
				else {
					cout << "Invalid command." << endl;
				}
			}
			break;

		case 7:
			cout << "Enter filename to save contacts: ";
			getline(cin, filename);
			directory.saveToFile(filename);
			break;

		case 8:
			cout << "Enter filename to load contacts: ";
			getline(cin, filename);
			directory.loadFromFile(filename);
			break;

		case 9:
			cout << "Exiting..." << endl;
			return 0;
			break;

		case 10:
			directory.saveToFile(filename);
			cout << "Saving and Exiting..." << endl;
			return 0;
			break;

		default:
			cout << "Invalid choice. Please try again." << endl;
			break;
		}

	} while (choice != 10);

	// Save contacts to file
	directory.saveToFile("contacts.txt");



	return 0;
}

